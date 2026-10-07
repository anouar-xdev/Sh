//Fuck Garena Fuck All 
//Damn your life if you share it without my name, you damned child! 
//Up- @XAnouar
//anwarxdevs.dpdns.org


#pragma once
#include <cstdint>

namespace Offsets {
constexpr uintptr_t GameFacade = 0xAD68E18;
constexpr uintptr_t GameFacade_P2 = 0xB8;
constexpr uintptr_t BaseGame_Match = 0x90;
constexpr uintptr_t CurrentObserve = 0x100;
constexpr uintptr_t ObserverPlayer = 0x30;
constexpr uintptr_t BaseGame_Timer = 0x20;
constexpr uintptr_t GhostHack = 0x814;
constexpr uintptr_t Match_PlayerDict = 0xC0;
constexpr uintptr_t Match_LocalPlayer = 0xD8;

constexpr uintptr_t Dict_Entries = 0x18;
constexpr uintptr_t Dict_Count = 0x20;
constexpr uintptr_t Dict_ArrayHeader = 0x20;
constexpr uintptr_t Dict_EntryStride = 0x18;
constexpr uintptr_t Dict_EntryValue = 0x10;

constexpr uintptr_t Player_Rotation = 0x61C;
constexpr uintptr_t Player_HeadTF = 0x6A8;
constexpr uintptr_t Player_FootTF = 0x6D0;
constexpr uintptr_t Player_Camera = 0x698;
constexpr uintptr_t Player_IsFiring = 0x850;
constexpr uintptr_t Player_HP = 0x70;
constexpr uintptr_t Player_IsDead = 0x7C;
constexpr uintptr_t Player_Avatar = 0x778;
constexpr uintptr_t Player_HedColider = 0x740;
constexpr uintptr_t Player_ColWrite = 0x80;
constexpr uintptr_t Player_DeathInfo = 0x2290;
constexpr uintptr_t Player_IsBot = 0x4A8;
constexpr uintptr_t Player_Name = 0x4A0;

constexpr uintptr_t Avatar_Uma = 0x138;
constexpr uintptr_t Uma_Visible = 0x101;
constexpr uintptr_t Uma_Data = 0x28;
constexpr uintptr_t UmaData_Skip = 0x81;

constexpr uintptr_t Camera_Follow = 0x30;
constexpr uintptr_t Camera_IntPtr = 0x10;
constexpr uintptr_t Camera_Matrix = 0x100;

constexpr uintptr_t HP_List = 0x10;
constexpr uintptr_t HP_Entry = 0x20;
constexpr uintptr_t HP_Value = 0x18;

constexpr uintptr_t Player_BulletInfo = 0xE58;
constexpr uintptr_t Bullet_SpawnPos = 0x4C;
constexpr uintptr_t Bullet_DirWrite = 0x40;

constexpr uintptr_t MainCameraFallback = 0x3F0;

constexpr uintptr_t WeaponInstance = 0x608;
constexpr uintptr_t ReloadInstance = 0x708;
constexpr uintptr_t InventoryManager = 0x748;
constexpr uintptr_t StatusStruct = 0x1B98;

constexpr uintptr_t gameFacade = GameFacade;
constexpr uintptr_t StaticGame = GameFacade_P2;
constexpr uintptr_t CurrentMatch = BaseGame_Match;
constexpr uintptr_t MatchIsRunning = 0xCC;
constexpr uintptr_t DictionaryEntities = Match_PlayerDict;
constexpr uintptr_t localPlayer = Match_LocalPlayer;
constexpr uintptr_t playerAttributes = 0x770;
constexpr uintptr_t m_LocalObserver = CurrentObserve;
constexpr uintptr_t m_TargetPlayer = ObserverPlayer;
constexpr uintptr_t m_LocalSpectator = 0x108;
constexpr uintptr_t TargeSpectPlayer = 0x60;
constexpr uintptr_t isClientBot = Player_IsBot;
constexpr uintptr_t AimRotation = Player_Rotation;
constexpr uintptr_t Player_SpineTF = 0x650;
constexpr uintptr_t Player_RootTF = 0x6B0;
constexpr uintptr_t IsPrepareAttack = Player_IsFiring;
constexpr uintptr_t Player_Data = Player_HP;
constexpr uintptr_t Player_isDead = Player_IsDead;
constexpr uintptr_t AvatarManager = Player_Avatar;
constexpr uintptr_t AvatarObj = Avatar_Uma;
constexpr uintptr_t Avatar_IsVisible = Uma_Visible;
constexpr uintptr_t Avatar_Data = Uma_Data;
constexpr uintptr_t Avatar_IsTeam = UmaData_Skip;
constexpr uintptr_t FollowCamera = Player_Camera;
constexpr uintptr_t CameraObj = Camera_Follow;
constexpr uintptr_t Player_ShadowBase = Player_DeathInfo;
constexpr uintptr_t Player_IsKnocked = 0x80;
constexpr uintptr_t Player_TransformType = 0x11DC;
constexpr uintptr_t LastAimingInfo = Player_BulletInfo;
constexpr uintptr_t hitPos = Bullet_SpawnPos;
constexpr uintptr_t firePos = Bullet_DirWrite;
constexpr uintptr_t HeadCollider = Player_HedColider;
constexpr uintptr_t Aimlock = Player_ColWrite;
constexpr uintptr_t WeaponObj = WeaponInstance;
constexpr uintptr_t WeaponData = 0x88;
constexpr uintptr_t WeaponRecoil = 0x18;

constexpr uintptr_t sAim1 = 0x848;
constexpr uintptr_t sAim2 = 0xE50;
constexpr uintptr_t sAim3 = 0x40;
constexpr uintptr_t sAim4 = 0xD0;
constexpr uintptr_t CallSetAimRotationCount = 0x818;

constexpr uintptr_t IsGrounded = 0x11C;
constexpr uintptr_t CurrentGroundState = 0xD0;
constexpr uintptr_t LastGroundState = 0x110;
constexpr uintptr_t Velocity = 0xF0;
constexpr uintptr_t BHNEFIPKOMG = 0x24;
constexpr uintptr_t InSnowSlideWayDashing = 0x1EA0;
constexpr uintptr_t m_RightOffset = 0xA0;
constexpr uintptr_t m_UpOffset = 0xA4;
constexpr uintptr_t FOVOffset = 0x84;
constexpr uintptr_t RunSpeedUpScale = 0x2C0;

constexpr uintptr_t Engine_BaseGamePtr = 0xB8;
constexpr uintptr_t Engine_Match = 0x90;
constexpr uintptr_t Engine_Dictionary = 0xC0;
constexpr uintptr_t Engine_EntityList = 0x18;
constexpr uintptr_t Engine_TimeService = 0x20;
constexpr uintptr_t Engine_TimeServiceSpeed = 0x2C;

constexpr uintptr_t Match_MatchRunning = 0xCC;
constexpr uintptr_t Match_LocalPlayer2 = 0xD8;

constexpr uintptr_t Cam_FollowCamera = 0x698;
constexpr uintptr_t Cam_CameraObject = 0x30;
constexpr uintptr_t Cam_RawCameraData = 0x10;
constexpr uintptr_t Cam_ViewMatrix = 0x100;

constexpr uintptr_t Transform_TransformToObj = 0x10;
constexpr uintptr_t Transform_ObjToIndex = 0x40;
constexpr uintptr_t Transform_ObjToMatrix = 0x38;
constexpr uintptr_t Transform_MatrixList = 0x18;
constexpr uintptr_t Transform_MatrixIndices = 0x20;
constexpr uintptr_t Transform_MatrixSize = 0x30;
constexpr uintptr_t Transform_IndexSize = 0x4;

constexpr uintptr_t Player_MainCamera = 0x3F0;
constexpr uintptr_t Player_WeaponInstance = 0x608;
constexpr uintptr_t Player_ReloadInstance = 0x708;
constexpr uintptr_t Player_AvatarManager = 0x778;
constexpr uintptr_t Player_StatusStruct = 0x1B98;
constexpr uintptr_t Player_IsSpectating = 0x7C;
constexpr uintptr_t Player_HeadCollider = 0x740;
constexpr uintptr_t Player_AimTargetWrite = 0x80;
constexpr uintptr_t Player_FiringState = 0x850;
constexpr uintptr_t Player_ViewRotation = 0x214;
constexpr uintptr_t Player_ClientBotCheck = 0x4A8;
constexpr uintptr_t Player_NamePtr = 0x4A0;
constexpr uintptr_t Player_InventoryManager = 0x6E0;
constexpr uintptr_t Player_AimingInfo = 0xE58;
constexpr uintptr_t Player_AimingInfoStartPos = 0x4C;
constexpr uintptr_t Player_AimingInfoDirection = 0x40;
constexpr uintptr_t Player_HealthChain1 = 0x70;
constexpr uintptr_t Player_HealthChain2 = 0x10;
constexpr uintptr_t Player_HealthChain3 = 0x20;
constexpr uintptr_t Player_HealthValue = 0x18;
constexpr uintptr_t Player_IsDead2 = 0x7C;
constexpr uintptr_t Player_HeadTF2 = 0x6A8;
constexpr uintptr_t Player_FootTF2 = 0x6D0;

constexpr uintptr_t Weapon_WeaponData = 0x28;
constexpr uintptr_t Weapon_RecoilValue = 0x18;
constexpr uintptr_t Weapon_FastReloadTrigger = 0x111;

constexpr uintptr_t Status_StatusData = 0x20;
constexpr uintptr_t Status_StateID = 0x10;

constexpr uintptr_t Avatar_UmaSimple = 0x138;
constexpr uintptr_t Avatar_IsVisible2 = 0x101;
constexpr uintptr_t Avatar_UmaData = 0x28;
constexpr uintptr_t Avatar_IsDead = 0x81;

constexpr uintptr_t Inventory_ActiveItem = 0xA0;
constexpr uintptr_t Inventory_WeaponComponent = 0x80;
constexpr uintptr_t Inventory_NoRecoil = 0x18;
constexpr uintptr_t Inventory_ItemData = 0x28;
constexpr uintptr_t Inventory_WeaponId = 0x40;

constexpr uintptr_t Bones_Head = 0x6A8;
constexpr uintptr_t Bones_Chest = 0x6B8;
constexpr uintptr_t Bones_Hip = 0x6B0;
constexpr uintptr_t Bones_Feet = 0x6D0;
constexpr uintptr_t Bones_Neck = 0x658;
constexpr uintptr_t Bones_Spine = 0x650;
constexpr uintptr_t Bones_LeftShoulder = 0x710;
constexpr uintptr_t Bones_RightShoulder = 0x718;
constexpr uintptr_t Bones_LeftHand = 0x728;
constexpr uintptr_t Bones_RightHand = 0x720;
constexpr uintptr_t Bones_LeftAnkle = 0x6E0;
constexpr uintptr_t Bones_RightAnkle = 0x6E8;
constexpr uintptr_t Bones_Breast = 0x6B8;
constexpr uintptr_t Bones_Root = 0x6B0;
constexpr uintptr_t Bones_LeftToe = 0x6E0;
constexpr uintptr_t Bones_RightToe = 0x6E8;
constexpr uintptr_t Bones_RightForeArm = 0x728;
constexpr uintptr_t Bones_LeftForeArm = 0x730;

constexpr uintptr_t PlayerAttributes = 0x770;
constexpr uintptr_t ShootNoReload = 0x111;
}

/*
#pragma once
#include <cstdint>

namespace Offsets {
    constexpr uintptr_t GameFacade = 0xAD68E18;
    constexpr uintptr_t GameFacade_P2 = 0xB8;
    constexpr uintptr_t BaseGame_Match = 0x90;
    constexpr uintptr_t CurrentObserve = 0x100;
    constexpr uintptr_t ObserverPlayer = 0x30;
    constexpr uintptr_t BaseGame_Timer = 0x20;
    constexpr uintptr_t GhostHack = 0x814;
    constexpr uintptr_t Match_PlayerDict = 0xC0;
    constexpr uintptr_t Match_LocalPlayer = 0xD8;

    constexpr uintptr_t Dict_Entries = 0x18;
    constexpr uintptr_t Dict_Count = 0x20;
    constexpr uintptr_t Dict_ArrayHeader = 0x20;
    constexpr uintptr_t Dict_EntryStride = 0x18;
    constexpr uintptr_t Dict_EntryValue = 0x10;

    constexpr uintptr_t Player_Rotation = 0x61C;
    constexpr uintptr_t Player_HeadTF = 0x6a8;
    constexpr uintptr_t Player_FootTF = 0x6d0;
    constexpr uintptr_t Player_Camera = 0x698;
    constexpr uintptr_t Player_IsFiring = 0x850;
    constexpr uintptr_t Player_HP = 0x70;
    constexpr uintptr_t Player_IsDead = 0x7C;
    constexpr uintptr_t Player_Avatar = 0x778;
    constexpr uintptr_t Player_HedColider = 0x740;
    constexpr uintptr_t Player_ColWrite = 0x80;
    constexpr uintptr_t Player_DeathInfo = 0x2290;
    constexpr uintptr_t Player_IsBot = 0x4a8;
    constexpr uintptr_t Player_Name = 0x4a0;

    constexpr uintptr_t Avatar_Uma = 0x138;
    constexpr uintptr_t Uma_Visible = 0x101;
    constexpr uintptr_t Uma_Data = 0x28;
    constexpr uintptr_t UmaData_Skip = 0x81;

    constexpr uintptr_t Camera_Follow = 0x30;
    constexpr uintptr_t Camera_IntPtr = 0x10;
    constexpr uintptr_t Camera_Matrix = 0x100;

    constexpr uintptr_t HP_List = 0x10;
    constexpr uintptr_t HP_Entry = 0x20;
    constexpr uintptr_t HP_Value = 0x18;

    constexpr uintptr_t Player_BulletInfo = 0xe58;
    constexpr uintptr_t Bullet_SpawnPos = 0x4C;
    constexpr uintptr_t Bullet_DirWrite = 0x40;

    constexpr uintptr_t MainCameraFallback = 0x3f0;

    constexpr uintptr_t WeaponInstance = 0x608;
    constexpr uintptr_t ReloadInstance = 0x708;
    constexpr uintptr_t InventoryManager = 0x748;
    constexpr uintptr_t StatusStruct = 0x1B98;

    constexpr uintptr_t gameFacade = GameFacade;
    constexpr uintptr_t StaticGame = GameFacade_P2;
    constexpr uintptr_t CurrentMatch = BaseGame_Match;
    constexpr uintptr_t MatchIsRunning = 0xCC;
    constexpr uintptr_t DictionaryEntities = Match_PlayerDict;
    constexpr uintptr_t localPlayer = Match_LocalPlayer;
    constexpr uintptr_t playerAttributes = 0x770;
    constexpr uintptr_t m_LocalObserver = CurrentObserve;
    constexpr uintptr_t m_TargetPlayer = ObserverPlayer;
    constexpr uintptr_t m_LocalSpectator = 0x108;
    constexpr uintptr_t TargeSpectPlayer = 0x60;
    constexpr uintptr_t isClientBot = Player_IsBot;
    constexpr uintptr_t AimRotation = Player_Rotation;
    constexpr uintptr_t Player_SpineTF = 0x650;
    constexpr uintptr_t Player_RootTF = 0x6B0;
    constexpr uintptr_t IsPrepareAttack = Player_IsFiring;
    constexpr uintptr_t Player_Data = Player_HP;
    constexpr uintptr_t Player_isDead = Player_IsDead;
    constexpr uintptr_t AvatarManager = Player_Avatar;
    constexpr uintptr_t AvatarObj = Avatar_Uma;
    constexpr uintptr_t Avatar_IsVisible = Uma_Visible;
    constexpr uintptr_t Avatar_Data = Uma_Data;
    constexpr uintptr_t Avatar_IsTeam = UmaData_Skip;
    constexpr uintptr_t FollowCamera = Player_Camera;
    constexpr uintptr_t CameraObj = Camera_Follow;
    constexpr uintptr_t Player_ShadowBase = Player_DeathInfo;
    constexpr uintptr_t Player_IsKnocked = 0x80;
    constexpr uintptr_t Player_TransformType = 0x11DC;
    constexpr uintptr_t LastAimingInfo = Player_BulletInfo;
    constexpr uintptr_t hitPos = Bullet_SpawnPos;
    constexpr uintptr_t firePos = Bullet_DirWrite;
    constexpr uintptr_t HeadCollider = Player_HedColider;
    constexpr uintptr_t Aimlock = Player_ColWrite;
    constexpr uintptr_t WeaponObj = WeaponInstance;
    constexpr uintptr_t WeaponData = 0x88;
    constexpr uintptr_t WeaponRecoil = 0x18;

    constexpr uintptr_t sAim1 = 0x848;
    constexpr uintptr_t sAim2 = 0xE50;
    constexpr uintptr_t sAim3 = 0x40;
    constexpr uintptr_t sAim4 = 0xD0;
    constexpr uintptr_t CallSetAimRotationCount = 0x79C;

    constexpr uintptr_t IsGrounded = 0x11C;
    constexpr uintptr_t CurrentGroundState = 0xD0;
    constexpr uintptr_t LastGroundState = 0x110;
    constexpr uintptr_t Velocity = 0xF0;
    constexpr uintptr_t BHNEFIPKOMG = 0x24;
    constexpr uintptr_t InSnowSlideWayDashing = 0x1EA0;
    constexpr uintptr_t m_RightOffset = 0x8C;
    constexpr uintptr_t m_UpOffset = 0x90;
    constexpr uintptr_t FOVOffset = 0x70;
    constexpr uintptr_t RunSpeedUpScale = 0x270;

    constexpr uintptr_t Engine_BaseGamePtr = 0xB8;
    constexpr uintptr_t Engine_Match = 0x90;
    constexpr uintptr_t Engine_Dictionary = 0xC0;
    constexpr uintptr_t Engine_EntityList = 0x18;
    constexpr uintptr_t Engine_TimeService = 0x20;
    constexpr uintptr_t Engine_TimeServiceSpeed = 0x2C;

    constexpr uintptr_t Match_MatchRunning = 0xCC;
    constexpr uintptr_t Match_LocalPlayer2 = 0xD8;

    constexpr uintptr_t Cam_FollowCamera = 0x698;
    constexpr uintptr_t Cam_CameraObject = 0x30;
    constexpr uintptr_t Cam_RawCameraData = 0x10;
    constexpr uintptr_t Cam_ViewMatrix = 0x100;

    constexpr uintptr_t Transform_TransformToObj = 0x10;
    constexpr uintptr_t Transform_ObjToIndex = 0x40;
    constexpr uintptr_t Transform_ObjToMatrix = 0x38;
    constexpr uintptr_t Transform_MatrixList = 0x18;
    constexpr uintptr_t Transform_MatrixIndices = 0x20;
    constexpr uintptr_t Transform_MatrixSize = 0x30;
    constexpr uintptr_t Transform_IndexSize = 0x4;

    constexpr uintptr_t Player_MainCamera = 0x3f0;
    constexpr uintptr_t Player_WeaponInstance = 0x608;
    constexpr uintptr_t Player_ReloadInstance = 0x708;
    constexpr uintptr_t Player_AvatarManager = 0x778;
    constexpr uintptr_t Player_StatusStruct = 0x1B98;
    constexpr uintptr_t Player_IsSpectating = 0x7C;
    constexpr uintptr_t Player_HeadCollider = 0x740;
    constexpr uintptr_t Player_AimTargetWrite = 0x80;
    constexpr uintptr_t Player_FiringState = 0x850;
    constexpr uintptr_t Player_ViewRotation = 0x214;
    constexpr uintptr_t Player_ClientBotCheck = 0x4a8;
    constexpr uintptr_t Player_NamePtr = 0x4a0;
    constexpr uintptr_t Player_InventoryManager = 0x6E0;
    constexpr uintptr_t Player_AimingInfo = 0xe58;
    constexpr uintptr_t Player_AimingInfoStartPos = 0x4C;
    constexpr uintptr_t Player_AimingInfoDirection = 0x40;
    constexpr uintptr_t Player_HealthChain1 = 0x70;
    constexpr uintptr_t Player_HealthChain2 = 0x10;
    constexpr uintptr_t Player_HealthChain3 = 0x20;
    constexpr uintptr_t Player_HealthValue = 0x18;
    constexpr uintptr_t Player_IsDead2 = 0x7C;
    constexpr uintptr_t Player_HeadTF2 = 0x6a8;
    constexpr uintptr_t Player_FootTF2 = 0x6d0;

    constexpr uintptr_t Weapon_WeaponData = 0x28;
    constexpr uintptr_t Weapon_RecoilValue = 0x18;
    constexpr uintptr_t Weapon_FastReloadTrigger = 0x111;

    constexpr uintptr_t Status_StatusData = 0x20;
    constexpr uintptr_t Status_StateID = 0x10;

    constexpr uintptr_t Avatar_UmaSimple = 0x138;
    constexpr uintptr_t Avatar_IsVisible2 = 0x101;
    constexpr uintptr_t Avatar_UmaData = 0x28;
    constexpr uintptr_t Avatar_IsDead = 0x81;

    constexpr uintptr_t Inventory_ActiveItem = 0xA0;
    constexpr uintptr_t Inventory_WeaponComponent = 0x80;
    constexpr uintptr_t Inventory_NoRecoil = 0x18;
    constexpr uintptr_t Inventory_ItemData = 0x28;
    constexpr uintptr_t Inventory_WeaponId = 0x40;

    constexpr uintptr_t Bones_Head = 0x6a8;
    constexpr uintptr_t Bones_Chest = 0x6b8;
    constexpr uintptr_t Bones_Hip = 0x6b0;
    constexpr uintptr_t Bones_Feet = 0x6d0;
    constexpr uintptr_t Bones_Neck = 0x658;
    constexpr uintptr_t Bones_Spine = 0x650;
    constexpr uintptr_t Bones_LeftShoulder = 0x710;
    constexpr uintptr_t Bones_RightShoulder = 0x718;
    constexpr uintptr_t Bones_LeftHand = 0x728;
    constexpr uintptr_t Bones_RightHand = 0x720;
    constexpr uintptr_t Bones_LeftAnkle = 0x6e0;
    constexpr uintptr_t Bones_RightAnkle = 0x6e8;
    constexpr uintptr_t Bones_Breast = 0x6b8;
    constexpr uintptr_t Bones_Root = 0x6b0;
    constexpr uintptr_t Bones_LeftToe = 0x6E0;
    constexpr uintptr_t Bones_RightToe = 0x6E8;
    constexpr uintptr_t Bones_RightForeArm = 0x728;
    constexpr uintptr_t Bones_LeftForeArm = 0x730;
    
    
   
constexpr uintptr_t PlayerAttributes = 0x770; 
constexpr uintptr_t ShootNoReload   = 0x111;
}

*/