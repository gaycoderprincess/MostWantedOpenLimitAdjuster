#include <windows.h>
#include <format>
#include <filesystem>
#include <toml++/toml.hpp>

#include "nya_commonhooklib.h"
#include "nfsmw.h"

void WriteLog(const std::string& str) {
	static auto file = std::ofstream("NFSMWOpenLimitAdjuster_gcp.log");

	file << str;
	file << "\n";
	file.flush();
}

void __thiscall NewVector_Destruct(UTL::Vector<void*>* pThis, int a2) {
	if ((a2 & 1) == 0) return;

	if (!pThis->mBegin) return;
	GAME_free(pThis->mBegin);
}

template<size_t count>
void* __thiscall NewVector_AllocVectorSpace(UTL::Vector<void*>* pThis, size_t num, size_t alignment) {
	//WriteLog(std::format("AllocVectorSpace {:X}", (uintptr_t)__builtin_return_address(0)));
	return GAME_malloc(num * count);
}

void __thiscall NewVector_FreeVectorSpace(UTL::Vector<void*>* pThis, void* buffer, size_t num) {
	return GAME_free(buffer);
}

size_t __thiscall NewVector_GetGrowSize(UTL::Vector<void*>* pThis, size_t minSize) {
	return std::max(minSize, pThis->mCapacity);
}

size_t __thiscall NewVector_GetMaxCapacity(UTL::Vector<void*>* pThis) {
	return 0x7FFFFFFF;
}

void __thiscall NewVector_OnGrowRequest(UTL::Vector<void*>* pThis, size_t newSize) {
	return;
}

void* aNewVectorVTable[] = {
	(void*)&NewVector_Destruct,
	(void*)&NewVector_AllocVectorSpace<4>,
	(void*)&NewVector_FreeVectorSpace,
	(void*)&NewVector_GetGrowSize,
	(void*)&NewVector_GetMaxCapacity,
	(void*)&NewVector_OnGrowRequest,
};

void* aNewGarbageVectorVTable[] = {
	(void*)&NewVector_Destruct,
	(void*)&NewVector_AllocVectorSpace<8>,
	(void*)&NewVector_FreeVectorSpace,
	(void*)&NewVector_GetGrowSize,
	(void*)&NewVector_GetMaxCapacity,
	(void*)&NewVector_OnGrowRequest,
};

void* aNewVehicleManagementNodeVectorVTable[] = {
	(void*)&NewVector_Destruct,
	(void*)&NewVector_AllocVectorSpace<0x18>,
	(void*)&NewVector_FreeVectorSpace,
	(void*)&NewVector_GetGrowSize,
	(void*)&NewVector_GetMaxCapacity,
	(void*)&NewVector_OnGrowRequest,
};

void BreakHooked() {
	MessageBoxA(nullptr, std::format("Breakpoint called from {:X}", (uintptr_t)__builtin_return_address(0)).c_str(), "nya?!~", MB_ICONERROR);
	__debugbreak();
}

const char* aSlotPoolNames[] = {
		"AnimCtrlSlotPool",
		"Anim_CAnimSkeleton_SlotPool",
		"Anim_CNFSAnimBank_SlotPool",
		"AStarNodeSlotPool",
		"AStarSearchSlotPool",
		"AUD_CsisSlotPools",
		"bFileSystem",
		"CACHE_SpeechEventslotpool",
		"CarEmitterPositionSlotPool",
		"CarEmitterPositionSlotPool",
		"CarLoadedRideInfoSlotPool",
		"CarLoadedSkinLayerSlotPool",
		"CarLoadedSolidPackSlotPool",
		"CarLoadedTexturePackSlotPool",
		"CarPartModelPool",
		"ClanSlotPool",
		"eAnimTextureSlotPool",
		"Ecstacy_ModelSlotPool",
		"eLightMaterialPlatInfoSlotPool",
		"eMeshRender",
		"EmitterGroupSlotPool",
		"EmitterSlotPool",
		"ePolySlotPool",
		"eStreamingPackSlotPool",
		"eStripSlotPool",
		"EventHandlerSlotPool",
		"EventSlotPool",
		"FERenderEPolySlotPool",
		"FERenderObjectSlotPool",
		"g_textureHeaderPool",
		"g_vertexBufferHeaderPool",
		"ParticleSlotPool",
		"QueuedFileSlotPool",
		"ResourceFileSlotPool",
		"SampleWrapperSlotPool",
		"ShadowMapMeshSlotPool",
		"SkidSetSlotPool",
		"SpaceNodeSlotPool",
		"StitchSlotPool",
		"TexturePackSlotPool",
		"VehicleDamagePartSlotPool",
		"VehiclePartDamageZoneSlotPool",
		"VoiceActorsslotpool",
		"WorldAnimCtrl_SlotPool",
		"WorldAnimEntityTreeInfo_SlotPool",
		"WorldAnimEntityTree_SlotPool",
		"WorldAnimEntity_SlotPool",
		"WorldAnimInstanceEntry_SlotPool",
		"WorldModelSlotPool",
};
size_t aSlotPoolSizes[sizeof(aSlotPoolNames)/sizeof(aSlotPoolNames[0])] = {};

SlotPool* bNewSlotPoolHooked(int slot_size, int num_slots, const char *debug_name, int memory_pool) {
	if (debug_name) {
		for (int i = 0; i < sizeof(aSlotPoolNames)/sizeof(aSlotPoolNames[0]); i++) {
			if (!aSlotPoolSizes[i]) continue;
			if (strcmp(debug_name, aSlotPoolNames[i])) continue;

			num_slots = aSlotPoolSizes[i];
		}
	}
	return bNewSlotPool(slot_size, num_slots, debug_name, memory_pool);
}

int nMaxSimpleRigidBodies = 96;
bool CanSpawnSimpleRigidBodyNew() {
	int count = SimpleRigidBody::mCount;
	if (count >= nMaxSimpleRigidBodies) {
		auto table = &DISPOSABLE_LIST::_mTable;
		for (int i = 0; i < table->size(); i++) {
			auto disposable = table->get(i);
			if (disposable->IsRequired()) continue;
			auto simable = disposable->mCOMObject->Find<ISimable>();
			if (!simable) continue;
			simable->Kill();
			count--;
		}
	}
	return count < nMaxSimpleRigidBodies;
}

auto SimplifySort_orig = (bool(*)(uintptr_t, uintptr_t))0x6709E0;
bool SimplifySortHooked(uintptr_t a1, uintptr_t a2) {
	if (a1 == 0x1 || a2 == 0x1) {
		MessageBoxA(0, std::format("Smackable check invalid at {:X} {:X}", a1, a2).c_str(), "nya?!~", MB_ICONERROR);

		auto table = &SMACKABLE_LIST::_mTable;
		for (int i = 0; i < table->size(); i++) {
			auto smackable = (uintptr_t)table->get(i);
			if (smackable == 0x1) {
				MessageBoxA(0, std::format("Smackable at id {} is invalid", i).c_str(), "nya?!~", MB_ICONERROR);
			}
		}
	}
	return SimplifySort_orig(a1, a2);
}

template<int elementSize>
void __thiscall VectorSizeHooked(uintptr_t* addr, int size) {
	WriteLog(std::format("Vector is size {} at vtable {:X}", elementSize, *addr));
	auto vec = (UTL::Vector<void*>*)addr;
	vec->mBegin = vec->AllocVectorSpace(size, 16);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH: {
			if (NyaHookLib::GetEntryPoint() != 0x3C4040) {
				MessageBoxA(nullptr, "Unsupported game version! Make sure you're using v1.3 (.exe size of 6029312 bytes)", "nya?!~", MB_ICONERROR);
				return TRUE;
			}

			NyaHookLib::Patch(0x68501B + 1, &SimplifySortHooked);
			//NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x41C080, &Size8Hooked);
			//NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x4C01D0, &Size8Hooked);
			//NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x4EA9B0, &Size14Hooked);
			//NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x4EAB00, &Size8Hooked);
			//NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x4EAD20, &Size8Hooked);
			//NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x674060, &Size18Hooked);
			//NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x6749E0, &Size8Hooked);
			//NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x6EF850, &Size8Hooked);
			//NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x6EF960, &Size8Hooked);
			//NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x6EFA70, &Size8Hooked);

			if (std::filesystem::exists("NFSMWOpenLimitAdjuster_gcp.toml")) {
				try {
					toml::parse_file("NFSMWOpenLimitAdjuster_gcp.toml");
				}
				catch (const toml::parse_error& err) {
					MessageBoxA(0, std::format("Failed to parse config: {}", err.what()).c_str(), "nya?!~", MB_ICONERROR);
				}

				auto config = toml::parse_file("NFSMWOpenLimitAdjuster_gcp.toml");
				CarLoaderPoolSizes = config["car_loader_memory"].value_or(CarLoaderPoolSizes);

				NyaHookLib::Patch<uint32_t>(0x64A56E, config["fastmem"].value_or(0x21F400));

				for (int i = 0; i < sizeof(aSlotPoolNames)/sizeof(aSlotPoolNames[0]); i++) {
					aSlotPoolSizes[i] = config["slot_pools"][aSlotPoolNames[i]].value_or(0);
				}

				auto ReplacementTextureTableFixupSize = config["replacement_texture_table"].value_or(182);
				auto aNewReplacementTextureTableFixups = new uint32_t[ReplacementTextureTableFixupSize*2];
				memset(aNewReplacementTextureTableFixups, 0, sizeof(uint32_t)*ReplacementTextureTableFixupSize*2);
				NyaHookLib::Patch(0x4FB607, &aNewReplacementTextureTableFixups[0]);
				NyaHookLib::Patch(0x4FB655, &aNewReplacementTextureTableFixups[0]);
				NyaHookLib::Patch(0x4FB698, &aNewReplacementTextureTableFixups[0]);
				NyaHookLib::Patch(0x5013E6, &aNewReplacementTextureTableFixups[0]);
				NyaHookLib::Patch(0x4FB625, &aNewReplacementTextureTableFixups[ReplacementTextureTableFixupSize*2]);
				NyaHookLib::Patch(0x4FB668, &aNewReplacementTextureTableFixups[ReplacementTextureTableFixupSize*2]);
				NyaHookLib::Patch(0x4FB6D4, &aNewReplacementTextureTableFixups[ReplacementTextureTableFixupSize*2]);
				NyaHookLib::Patch(0x50145E, &aNewReplacementTextureTableFixups[ReplacementTextureTableFixupSize*2]);

				auto rigidBodyCount = config["rigidbody_count"].value_or(64);
				nMaxSimpleRigidBodies = config["simplerigidbody_count"].value_or(96);

				auto aVolatilePtrs = new void*[rigidBodyCount];
				auto aVolatileWorkspace = new uint8_t[rigidBodyCount*0xB0];
				auto aRigidBodyMaps = new int[rigidBodyCount];
				auto aSimpleVolatilePtrs = new void*[nMaxSimpleRigidBodies];
				auto aSimpleVolatileWorkspace = new uint8_t[nMaxSimpleRigidBodies*0x40];
				auto aSimpleRigidBodyMaps = new int[nMaxSimpleRigidBodies];

				memset(aVolatilePtrs, 0, sizeof(void*)*rigidBodyCount);
				memset(aVolatileWorkspace, 0, rigidBodyCount*0xB0);
				memset(aRigidBodyMaps, 0, sizeof(int)*rigidBodyCount);
				memset(aSimpleVolatilePtrs, 0, sizeof(void*)*nMaxSimpleRigidBodies);
				memset(aSimpleVolatileWorkspace, 0, nMaxSimpleRigidBodies*0x40);
				memset(aSimpleRigidBodyMaps, 0, sizeof(int)*nMaxSimpleRigidBodies);

				NyaHookLib::Patch(0x6959D5, &aVolatilePtrs[0]);
				NyaHookLib::Patch(0x6959EF, &aVolatilePtrs[0]);
				NyaHookLib::Patch(0x6A6453, &aVolatilePtrs[0]);
				NyaHookLib::Patch(0x6A647E, &aVolatilePtrs[0]);
				*(void**)0x9383A8 = aVolatileWorkspace; // ScratchPtr<RigidBody::Volatile>::mWorkSpace

				NyaHookLib::Patch<uint8_t>(0x68BA34 + 2, rigidBodyCount);
				NyaHookLib::Patch<uint8_t>(0x68BA5E + 2, rigidBodyCount);
				NyaHookLib::Patch<uint8_t>(0x6B5C6F + 2, rigidBodyCount);
				NyaHookLib::Patch<uint8_t>(0x6A645A + 2, rigidBodyCount);
				// just remove the bound checks above 255, i cba right now
				if (rigidBodyCount > 0xFF) {
					NyaHookLib::Patch<uint16_t>(0x68BA37, 0x9090);
					NyaHookLib::Patch<uint8_t>(0x68BA61, 0xEB);
					NyaHookLib::Patch<uint8_t>(0x6B5C72, 0xEB);
					NyaHookLib::Patch<uint8_t>(0x6A645D, 0xEB);
				}

				NyaHookLib::Patch(0x68BA3C, &aRigidBodyMaps[0]);
				NyaHookLib::Patch(0x68BA55, &aRigidBodyMaps[0]);
				NyaHookLib::Patch(0x6B5C68, &aRigidBodyMaps[0]);
				NyaHookLib::Patch(0x6B5F95, &aRigidBodyMaps[0]);
				NyaHookLib::Patch(0x6B77C4, &aRigidBodyMaps[0]);

				NyaHookLib::Patch<uint8_t>(0x68BBE4 + 2, nMaxSimpleRigidBodies);
				NyaHookLib::Patch<uint8_t>(0x6B60CA + 2, nMaxSimpleRigidBodies);
				NyaHookLib::Patch<uint8_t>(0x68BC0E + 2, nMaxSimpleRigidBodies);
				NyaHookLib::Patch<uint8_t>(0x6A64CC + 2, nMaxSimpleRigidBodies);
				// just remove the bound checks above 255, i cba right now
				if (nMaxSimpleRigidBodies > 0xFF) {
					NyaHookLib::Patch<uint16_t>(0x68BBE7, 0x9090);
					NyaHookLib::Patch<uint8_t>(0x6B60CD, 0xEB);
					NyaHookLib::Patch<uint8_t>(0x68BC11, 0xEB);
					NyaHookLib::Patch<uint8_t>(0x6A64CF, 0xEB);
				}

				NyaHookLib::Patch(0x695A15, &aSimpleVolatilePtrs[0]);
				NyaHookLib::Patch(0x695A2F, &aSimpleVolatilePtrs[0]);
				NyaHookLib::Patch(0x6A64C3, &aSimpleVolatilePtrs[0]);
				NyaHookLib::Patch(0x6A64E0, &aSimpleVolatilePtrs[0]);
				*(void**)0x93839C = aSimpleVolatileWorkspace; // ScratchPtr<SimpleRigidBody::Volatile>::mWorkSpace

				NyaHookLib::Patch(0x68BBEC, &aSimpleRigidBodyMaps[0]);
				NyaHookLib::Patch(0x68BC05, &aSimpleRigidBodyMaps[0]);
				NyaHookLib::Patch(0x6B60C3, &aSimpleRigidBodyMaps[0]);
				NyaHookLib::Patch(0x6B617B, &aSimpleRigidBodyMaps[0]);
				NyaHookLib::Patch(0x6B65EF, &aSimpleRigidBodyMaps[0]);

				// Sim::CanSpawnRigidBody
				NyaHookLib::Patch<uint16_t>(0x6ED26F, 0xC031); // xor eax, eax
				NyaHookLib::Patch<uint8_t>(0x6ED271, 0xB8); // mov eax,
				NyaHookLib::Patch<uint32_t>(0x6ED272, rigidBodyCount);
				NyaHookLib::Patch<uint8_t>(0x6ED276, 0x90); // nop

				NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x6ED390, &CanSpawnSimpleRigidBodyNew);
			}

			NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x45CD20, &BreakHooked);

			// PVehicle::MakeRoom increase to 127
			NyaHookLib::Patch<uint8_t>(0x687817 + 2, 0x7F); // cmp eax,7F
			NyaHookLib::Patch<uint8_t>(0x68781C + 2, 0x81); // lea edx,[eax-7F]

			// SFXObj_MomentStrm::stMomentDecription size 0x14
			/*for (int i = 0; i < 6; i++) {
				NyaHookLib::Patch(0x899200 + (i * 4), aVTable[i]);
			}*/

			for (int i = 0; i < 6; i++) {
				NyaHookLib::Patch(0x8AA234 + (i * 4), aNewVehicleManagementNodeVectorVTable[i]);
			}

			uintptr_t garbagevtables[] = {
				0x899218,
				0x899248,
				0x8A7E98,
				0x8AA038, // UTL::Collections::GarbageNode<PhysicsObject,160>::_mCollector
				0x8AA2D4,
				0x8AA2EC,
				0x8AA304,
				0x8AA3C4,
				0x8B06E8,
				0x8B0700,
				0x8B0718,
			};
			for (auto& addr : garbagevtables) {
				for (int i = 0; i < 6; i++) {
					NyaHookLib::Patch(addr + (i * 4), aNewGarbageVectorVTable[i]);
				}
			}

			uintptr_t vtables[] = {
				0x89157C, // unknown 20
				0x8915B8, // unknown 20
				0x891898, // unknown 20
				0x8918D8, // IVehicle 20
				0x897774, // unknown 20
				0x89778C, // unknown 20
				0x8977A4, // CarSoundConn 20
				0x8977BC, // unknown 20
				0x8977D4, // unknown 25
				0x8A9EA0, // unknown 160
				0x8A9EB8, // unknown 40
				0x8A9ED0, // unknown 2296
				0x8A9EE8, // unknown 96
				0x8A9F00, // unknown 8
				0x8A9F18, // unknown 28
				0x8A9F30, // unknown 160
				0x8A9F48, // unknown 96
				0x8A9F60, // unknown 160
				0x8A9F78, // unknown 20
				0x8A9F90, // unknown 8
				0x8A9FA8, // unknown 8
				0x8A9FC0, // unknown 8
				0x8A9FD8, // unknown 160
				0x8A9FF0, // unknown 8
				0x8AA008, // unknown 20
				0x8AA020, // unknown 2
				0x8AA050, // unknown 160
				0x8AA068, // unknown 8
				0x8AA080, // unknown 8
				0x8AA31C, // unknown 2296
				0x8AA334, // unknown 96
				0x8AA34C, // unknown 8
				0x8AA364, // IVehicleCache 28
				0x8AA37C, // unknown 160
				0x8AA394, // ISimpleBody 96
				0x8AA3AC, // unknown 160
				0x8AA3DC, // unknown 8
				0x8AA3F4, // unknown 8
				0x8AA40C, // unknown 8
				0x8AA424, // IDisposable 160
				0x8AA43C, // unknown 8
				0x8AA454, // unknown 20
				0x8AA46C, // unknown 2
				0x8AA484, // Smackable 160
				0x8AA49C, // IPlayer 8
				0x8AA4B4, // unknown 8
				0x8B51A8, // VehicleRenderConn 20
			};
			for (auto& addr : vtables) {
				for (int i = 0; i < 6; i++) {
					NyaHookLib::Patch(addr + (i * 4), aNewVectorVTable[i]);
				}
			}

			uintptr_t slotpools[] = {
					0x4454B4,
					0x445504,
					0x445C1E,
					0x447654,
					0x4476A4,
					0x447B04,
					0x447B54,
					0x449169,
					0x44B2E5,
					0x44D556,
					0x44D568,
					0x44D57B,
					0x44D591,
					0x44D5A7,
					0x4C4124,
					0x4CAA6A,
					0x4CAA91,
					0x4FB1D9,
					0x4FB81E,
					0x4FBF12,
					0x4FCD0E,
					0x4FE34E,
					0x4FE3A1,
					0x4FE401,
					0x4FF0AE,
					0x4FF0DB,
					0x4FF108,
					0x500A25,
					0x56F92E,
					0x56F949,
					0x56F9ED,
					0x5826AD,
					0x5828BD,
					0x582ABD,
					0x591893,
					0x591B08,
					0x591B23,
					0x64ABAB,
					0x64CE6E,
					0x64D20E,
					0x64D8DE,
					0x65F6B4,
					0x6BE311,
					0x6BEA6E,
					0x6C08AE,
					0x6DC465,
					0x6E42BE,
					0x6E42DF,
					0x6E6A8C,
					0x6E6AB0,
					0x6E6DEF,
					0x70F5AF,
					0x720538,
					0x7222AA,
					0x7223E4,
					0x72570B,
					0x725720,
					0x7376CE,
					0x73783F,
					0x73960E,
					0x73A35B,
					0x73A370,
					0x73A388,
					0x73A3A0,
					0x73B9B1,
					0x73EB2E,
					0x73EB43,
					0x773D64,
					0x773D79,
			};
			for (auto& addr : slotpools) {
				NyaHookLib::PatchRelative(NyaHookLib::CALL, addr, &bNewSlotPoolHooked);
			}
		} break;
		default:
			break;
	}
	return TRUE;
}