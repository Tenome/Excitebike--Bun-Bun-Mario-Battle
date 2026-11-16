/*
 * Routines identical (instruction for instruction) to S4's hand-written
 * ones, carried over by tools/port_hand.py with their return addresses
 * moved. Checked by tools/difftest816.py on s3.
 */
#include "hand/hand.h"
#include "roster.h"
#include "ram.h"

enum { BSX_STATUS = 0x2190, BSX_CTRL = 0x2191, BSX_DATA = 0x2192, BSX_ERR = 0x2193,
       BSX_RESET = 0x2197, BSX_CHANNEL = 0x218E };
#define ARG_LEN     DP(0x00)    /* 16-bit byte count */
#define ARG_SRC     DP(0x02)    /* 16-bit source address */
#define ARG_SRCBANK DP(0x04)    /* source bank */

unsigned g_unpack_cycles;

static inline u32 q(u16 off, u16 field) { return (0x7F0000u + field + off) & 0xFFFFFF; }

void AccelerateRider_m1x1(void);
void Add32_m1x1(void);
void AddBcdDigits_m1x1(void);
void AddBonusPoints_m1x1(void);
void AddScoreLong_m1x1(void);
void AddScore_m1x1(void);
void AddSpeedToXLong_m1x1(void);
void AddSpeedToX_m1x1(void);
void AddStadiumBonus_m1x1(void);
void AimRatio_m1x1(void);
void AimVelocity_m1x1(void);
void ApplyVelocityX_m1x1(void);
void ApplyVelocity_m1x1(void);
void BallHit_m1x1(void);
void BallOrGauge_m1x1(void);
void BeginFadeIn_m1x1(void);
void BestTimeBlink_m1x1(void);
void BlankQueueRowB_m1x1(void);
void BlankQueueRowPal2_m1x1(void);
void BlankQueueRow_m1x1(void);
void BonusCountUp_m1x1(void);
void BonusCourseFrame_m1x1(void);
void BonusResultStep_m1x1(void);
void BonusRowList_m1x1(void);
void BonusScrollStep_m1x1(void);
void BonusSignStep_m1x1(void);
void BonusTimerStep_m1x1(void);
void BoostOnTile_m1x1(void);
void BrakeOnTurn_m1x1(void);
void BrakeRider_m1x1(void);
void BroadcastEndScreens_m1x1(void);
void BroadcastTotals_m1x1(void);
void BsxDetect_m1x1(void);
void BsxEndIfFlagged_m1x1(void);
void BsxFindFile_m0x0(void);
void BsxOpenTimeChannel_m1x1(void);
void BsxPollStream_m1x1(void);
void BsxProbeReceiver_m1x0(void);
void BsxReadHeader3_m1x1(void);
void BsxReadSerialByte_m1x1(void);
void BuildBonusColumn_m1x1(void);
void BuildCourseColumn_m1x1(void);
void BuildJumpScore_m1x1(void);
void BuildScoreDigitsLong_m0x1(void);
void BuildScoreDigits_m0x1(void);
void ByteTable84BA_m1x1(void);
void CarouselPos_m1x1(void);
void ChangeModeLong_m1x1(void);
void ChangeMode_m1x1(void);
void CheckBonusTime_m1x1(void);
void CheckCourseTime_m1x1(void);
void CheckLaneTile_m1x1(void);
void ClearJumpFlags_m1x1(void);
void ClearObjectFrame_m1x1(void);
void ClearOnBonusTiles_m1x1(void);
void ClearRaceState_m1x1(void);
void ClearRiderAction_m1x1(void);
void ClearTilemaps_m1x1(void);
void ClearWords1530_m1x1(void);
void CoinMilestone_m1x1(void);
void CollectCoin_m1x1(void);
void CollectPickupPiece_m1x1(void);
void ComputePlaces_m1x1(void);
void ConfettiAll_m1x1(void);
void ConfettiStep_m1x1(void);
void CopyPieceColumnPal2_m1x1(void);
void CopyPieceColumn_m1x1(void);
void CopyTrackColumn_m1x1(void);
void CountCourseBits_m1x1(void);
void CourseColumnB_m1x1(void);
void CourseColumn_m1x1(void);
void CourseMinuteCheck_m1x1(void);
void CourseRowList_m1x1(void);
void CourseSelectPreview_m1x1(void);
void CpuAvoidHurdle_m1x1(void);
void CpuLaneDecision_m1x1(void);
void CpuReleaseButtons_m1x1(void);
void CpuReleaseDpad_m1x1(void);
void CpuRidersStep_m1x1(void);
void CpuSteerDown_m1x1(void);
void CpuSteerUp_m1x1(void);
void CpuSteer_m1x1(void);
void CpuThrottle_m1x1(void);
void CrashLand_m1x1(void);
void CrashRecovery_m1x1(void);
void CrossFinish_m1x1(void);
void DemoExitBoost_m1x1(void);
void DemoExitSkip35_m1x1(void);
void DemoExitSkip40_m1x1(void);
void DemoInputStep_m1x1(void);
void DemoPadDisplay_m1x1(void);
void DemoStepDispatch_m1x1(void);
void DispatchBy85B_m1x1(void);
void DispatchBy85_m1x1(void);
void DispatchByDistance_m1x1(void);
void DivideAX_m1x1(void);
void DmaFillZero_m0x1(void);
void DmaKick_m0x1(void);
void DrawAllRidersLong_m1x1(void);
void DrawAllRiders_m1x1(void);
void DrawBigBall_m1x1(void);
void DrawBigSprite_m1x1(void);
void DrawBonusPickup_m1x1(void);
void DrawClockB_m1x1(void);
void DrawCoinPickup_m1x1(void);
void DrawCountdownLights_m1x1(void);
void DrawCountdownSignB_m1x1(void);
void DrawCountdownSign_m1x1(void);
void DrawDustPair_m1x1(void);
void DrawJumpCrowd_m1x1(void);
void DrawJumpRank_m1x1(void);
void DrawJumpRiderSprites_m1x1(void);
void DrawObjectPieces_m1x1(void);
void DrawRaceHud_m1x1(void);
void DrawRiderMarker_m1x1(void);
void DrawRiderShadowPairLong_m1x1(void);
void DrawRiderShadowPair_m1x1(void);
void DrawRiderShadow_m1x1(void);
void DrawRider_m1x1(void);
void DrawShadowsAndMarkers_m1x1(void);
void DrawSpark_m1x1(void);
void DrawStar_m1x1(void);
void DrawStartFlags_m1x1(void);
void DriftUpObject_m1x1(void);
void Empty9A26_m1x1(void);
void EmptyB928_m1x1(void);
void EmptyBB45_m1x1(void);
void EmptyF03A_m1x1(void);
void EmptyHook_m1x1(void);
void EmptyStep9554_m1x1(void);
void EmptyStep9828_m1x1(void);
void EmptyVector_m1x1(void);
void EndAtBonusMinute_m1x1(void);
void EndJumpMode_m1x1(void);
void EndScreenRecord_m1x1(void);
void EndScreenScore_m1x1(void);
void EndScreenStart_m1x1(void);
void EndScreenTimes_m1x1(void);
void EnterNumberStep_m1x1(void);
void EntryConfirmStep_m1x1(void);
void EntryIntroStep_m1x1(void);
void EntryScreenFill_m1x1(void);
void FadeApuVolume_m1x1(void);
void FadeObjectStep_m1x1(void);
void FadePalette_m1x1(void);
void FadeSkyColours_m1x1(void);
void FallingObjectStep_m1x1(void);
void FillBonusColumns_m1x1(void);
void FillLaneColumn_m1x1(void);
void FindObjectSlot_m1x1(void);
void FinishModeSetup_m1x1(void);
void FlagRidersAtLine_m1x1(void);
void FlattenPlayerPiece_m1x1(void);
void ForceBlank_m1x1(void);
void FrameSequenceA_m1x1(void);
void FrameSequenceB_m1x1(void);
void GroundHeightAbove_m1x1(void);
void GrowBar087E_m1x1(void);
void HeightAbove_m1x1(void);
void HeightBelow_m1x1(void);
void HideAllSprites_m1x1(void);
void Hundreds_m0x1(void);
void Hundreds_m1x1(void);
void InitPpuRegisters_m1x1(void);
void IsPieceFlat_m1x1(void);
void ItemFlashState_m1x1(void);
void JumpBase10_m1x1(void);
void JumpBase30_m1x1(void);
void JumpContestFrame_m1x1(void);
void JumpContestMode_m1x1(void);
void JumpDistanceStepNear_m1x1(void);
void JumpDistanceStep_m1x1(void);
void JumpHudNumbers_m1x1(void);
void JumpPitchStep_m1x1(void);
void JumpResultBoardStep_m1x1(void);
void JumpResultBoard_m1x1(void);
void JumpResultBonus_m1x1(void);
void JumpResultCount_m1x1(void);
void JumpResultDistance_m1x1(void);
void JumpResultSkip_m1x1(void);
void JumpResultTally_m1x1(void);
void JumpResultTotal_m1x1(void);
void JumpResultWait_m1x1(void);
void JumpRiderStep_m1x1(void);
void JumpRunStart_m1x1(void);
void JumpRunSteps_m1x1(void);
void JumpRunUp_m1x1(void);
void JumpRun_m1x1(void);
void JumpTotalStep_m1x1(void);
void LandFlat_m1x1(void);
void LandingStep_m1x1(void);
void LaneFromY_m1x1(void);
void LanePieceAt_m1x1(void);
void LanePieceTablesLong_m1x1(void);
void LanePieceTables_m1x1(void);
void LaunchBall_m1x1(void);
void LeaveJumpContest_m1x1(void);
void LoadCourse_m1x1(void);
void LoadEntryScreenStep_m1x1(void);
void LoadHdmaScene_m1x1(void);
void LoadPackNumber_m1x1(void);
void LoadRaceColumn_m1x1(void);
void LoadRiderPalettes_m1x1(void);
void LoadScene_m1x1(void);
void LoadStadiumPalette_m1x1(void);
void MarkFastRider_m1x1(void);
void MarkerObjectStep_m1x1(void);
void MarkerObjects_m1x1(void);
void MergeBsxFlags_m1x1(void);
void MirrorJumpRider_m1x1(void);
void Mode00_IntroWait_m1x1(void);
void Mode02_SegmentCard_m1x1(void);
void Mode03_Race_m1x1(void);
void Mode09_BonusIntro_m1x1(void);
void Mode0A_m1x1(void);
void Mode0C_m1x1(void);
void Mode0E_TitleWait_m1x1(void);
void Mode0F_EndScreen_m1x1(void);
void Mode11_m1x1(void);
void Mode12_AfterEnd_m1x1(void);
void Mode17_BonusIntro_m1x1(void);
void ModeNop_m1x1(void);
void ModeStepNext_m1x1(void);
void MoveAndScroll_m1x1(void);
void MoveObjectX8_m1x1(void);
void MoveObjectX_m1x1(void);
void Mul16_m1x1(void);
void Mul16b_m1x0(void);
void Mul16b_m1x1(void);
void NextColumnLong_m1x1(void);
void NextColumn_m1x1(void);
void NextMode_m1x1(void);
void NmiFlushVramQueue_m1x1(void);
void NmiVector_m1x1(void);
void NmiVramUpload_m1x1(void);
void Nop_BD39_m1x1(void);
void ObjectAdvance20_m1x1(void);
void ObjectAdvance80_m1x1(void);
void ObjectAdvance8_m1x1(void);
void ObjectAdvance9_m1x1(void);
void ObjectAdvanceC0_m1x1(void);
void ObjectAdvanceOr4_m1x1(void);
void ObjectAdvanceOr5_m1x1(void);
void ObjectAdvanceRandom09_m1x1(void);
void ObjectAdvanceRandom0A_m1x1(void);
void ObjectAdvanceRandom10B_m1x1(void);
void ObjectAdvanceRandom10_m1x1(void);
void ObjectAdvanceRandom16_m1x1(void);
void ObjectAdvanceRandom20_m1x1(void);
void ObjectBlinkA_m1x1(void);
void ObjectBlinkB_m1x1(void);
void ObjectBlinkC_m1x1(void);
void ObjectBlinkD_m1x1(void);
void ObjectCycleA_m1x1(void);
void ObjectCycleB_m1x1(void);
void ObjectFacing_m1x1(void);
void ObjectFlicker_m1x1(void);
void ObjectFollowPath_m1x1(void);
void ObjectLoop5_m1x1(void);
void ObjectLoop7_m1x1(void);
void ObjectRepeat4A_m1x1(void);
void ObjectRepeat4B_m1x1(void);
void ObjectRepeat4Rest_m1x1(void);
void ObjectRun4_m1x1(void);
void ObjectRun6_m1x1(void);
void ObjectRunOnce6_m1x1(void);
void ObjectState9835_m1x1(void);
void ObjectStateB8E3_m1x1(void);
void ObjectStateB95D_m1x1(void);
void ObjectStateBA4C_m1x1(void);
void ObjectStateBB80_m1x1(void);
void ObjectStateBCC1_m1x1(void);
void ObjectStateBDDC_m1x1(void);
void ObjectStateBF34_m1x1(void);
void ObjectStateBFFC_m1x1(void);
void ObjectStepDispatch_m1x1(void);
void ObjectWalkCycle_m1x1(void);
void OffTrackBrake_m1x1(void);
void OverheatStep_m1x1(void);
void PackSpriteSizes_m1x1(void);
void PairHeight2_m1x1(void);
void PairHeight5_m1x1(void);
void PairJump6_m1x1(void);
void ParallaxLines_m1x1(void);
void ParkJumpRider_m1x1(void);
void PieceAction_m1x1(void);
void PitchDown8_m1x1(void);
void PitchHold_m1x1(void);
void PitchUp4_m1x1(void);
void PitchUp8_m1x1(void);
void PlaceAllRiderShadows_m1x1(void);
void PlaceLastRival_m1x1(void);
void PlaceMinuteMarker_m1x1(void);
void PlaceRiderShadowLong_m1x1(void);
void PlaceRiderShadow_m1x1(void);
void PlaceRival_m1x1(void);
void PlayerColumnCheckB_m1x1(void);
void PlayerColumnCheck_m1x1(void);
void PlayerCrashStepB_m1x1(void);
void PlayerCrashStep_m1x1(void);
void PlayerHaltB_m1x1(void);
void PlayerHalt_m1x1(void);
void PlayerSpeedBoost_m1x1(void);
void PlayerStopB_m1x1(void);
void PlayerStop_m1x1(void);
void QueueBestTimeLong_m1x1(void);
void QueueBestTime_m1x1(void);
void QueueBlinkBanner_m1x1(void);
void QueueBonusCount_m1x1(void);
void QueueBonusDigitsAt_m1x1(void);
void QueueBonusDigitsLong_m1x1(void);
void QueueBonusDigits_m1x1(void);
void QueueDigitsAt_m1x1(void);
void QueueEntryDigits_m1x1(void);
void QueueFiveDigits_m1x1(void);
void QueueFontDigits_m1x1(void);
void QueueFontTime_m1x1(void);
void QueueHalfUnits_m0x1(void);
void QueueMapTile_m1x1(void);
void QueueNameplate_m1x1(void);
void QueuePlaceBadgeHud_m1x1(void);
void QueuePlaceBadge_m1x1(void);
void QueuePlayerMarkers_m1x1(void);
void QueueRaceTime_m1x1(void);
void QueueResultsCount_m1x1(void);
void QueueResultsRow_m1x1(void);
void QueueResultsTiles_m1x1(void);
void QueueSoundB_m0x1(void);
void QueueSoundB_m1x1(void);
void QueueStadiumBadge_m1x1(void);
void QueueStadiumLabel_m1x1(void);
void QueueTileBlockLong_m1x1(void);
void QueueTileBlock_m1x1(void);
void QueueTileRowsLong_m1x1(void);
void QueueTileRows_m1x1(void);
void QueueTimeDigits_m1x1(void);
void RaceClockStep_m1x1(void);
void RaceFinishStep_m1x1(void);
void RaceFrameStep_m1x1(void);
void RaceMoveStep_m1x1(void);
void RacePlaceRiders_m1x1(void);
void RaceResultsSetup_m1x1(void);
void RaceRowList_m1x1(void);
void RaceStartLights_m1x1(void);
void RaceTimer_m1x1(void);
void RandomB_m1x0(void);
void RandomB_m1x1(void);
void RankPlayer_m1x1(void);
void Reach20_m1x1(void);
void Reach2_m1x1(void);
void ReadBroadcastDate_m1x1(void);
void ReadJoypads_m1x1(void);
void ReadJumpTrack_m1x1(void);
void ReadRiderPieces_m1x1(void);
void ReadStreamByte_m1x0(void);
void ReadStreamWord_m1x0(void);
void RecordCourseTime_m1x1(void);
void ReleaseBalloons_m1x1(void);
void ResetCoinCounts_m1x1(void);
void ResetDemoCourse_m1x1(void);
void ResetMinuteFrame_m1x1(void);
void ResetRaceBoard_m1x1(void);
void ResetSoundState_m1x1(void);
void ResetVector_m1x1(void);
void ResultsBonusCount_m1x1(void);
void ResultsJumpPoints_m0x1(void);
void ResultsPlacePoints_m0x1(void);
void ResultsWait_m1x1(void);
void RetargetRival_m1x1(void);
void RiderAhead_m1x1(void);
void RiderAtFinish_m1x1(void);
void RiderAtStart_m1x1(void);
void RiderBumpCrash_m1x1(void);
void RiderCollisions_m1x1(void);
void RiderDropStep_m1x1(void);
void RiderGfxCycle_m1x1(void);
void RiderGroundHeightsFrom_m1x1(void);
void RiderGroundHeights_m1x1(void);
void RiderHeightStep_m1x1(void);
void RiderJumpBase_m1x1(void);
void RiderLandCheck_m1x1(void);
void RiderLanding_m1x1(void);
void RiderLaneBand_m1x1(void);
void RiderLanePieceAt_m1x1(void);
void RiderOnFlat_m1x1(void);
void RiderOnRamp_m1x1(void);
void RiderPairHeight_m1x1(void);
void RiderPieceAt_m1x1(void);
void RiderPose_m1x1(void);
void RiderRunOut_m1x1(void);
void RiderShadowY_m1x1(void);
void RiderSpriteSets_m1x1(void);
void RiderStartLane_m1x1(void);
void RiderStateStep_m1x1(void);
void RiderStepAll_m1x1(void);
void RiderThrottleLong_m1x1(void);
void RiderThrottle_m1x1(void);
void RidersThrottle_m1x1(void);
void RivalEngineSound_m1x1(void);
void RivalTarget_m1x1(void);
void RunAllRiderPieces_m1x1(void);
void RunGameModeScript_m1x1(void);
void RunPieceActions_m1x1(void);
void RunRiderPieces_m1x1(void);
void SaveBestRecords_m1x1(void);
void SaveChecksum_m1x1(void);
void SaveHeaderCheck_m1x1(void);
void ScaleByScroll_m0x0(void);
void ScaleByScroll_m0x1(void);
void ScaleJumpCount_m0x1(void);
void ScaleJumpCount_m1x1(void);
void ScreenOff_m1x1(void);
void ScrollByPlayerSpeedLong_m1x1(void);
void ScrollByPlayerSpeed_m1x1(void);
void ScrollBySpeed16B_m1x1(void);
void ScrollBySpeed16_m1x1(void);
void ScrollDelta_m1x1(void);
void ScrollJumpRider_m1x1(void);
void ScrollRivalsRelative_m1x1(void);
void SelectDraw_m1x1(void);
void SelectHideRest_m1x1(void);
void SelectMain_m1x1(void);
void SelectName_m1x1(void);
void SelectRotate_m1x1(void);
void SelectSlideY_m1x1(void);
void SelectTimer_m1x1(void);
void SelectTransition_m1x1(void);
void SendSpeedToApu_m1x1(void);
void SetBgTileBase_m1x1(void);
void SetJumpBanner_m1x1(void);
void SetJumpBase_m1x1(void);
void SetJumpKind_m1x1(void);
void SetJumpRiderSprites_m1x1(void);
void SetJumpScroll_m1x1(void);
void SetJumpState1_m1x1(void);
void SetRaceBgMode_m1x1(void);
void SetRiderLean_m1x1(void);
void SetRiderPair_m1x1(void);
void SetupBonusScreen_m1x1(void);
void SetupCourseScreen_m1x1(void);
void SetupHdmaChannels_m1x1(void);
void SetupHdmaFlag_m1x1(void);
void SetupJumpContest_m1x1(void);
void SetupJumpScreen_m1x1(void);
void SetupSpriteRow15_m1x1(void);
void ShowBonusPointsLong_m1x1(void);
void ShowBonusPoints_m1x1(void);
void ShowCoinRank_m1x1(void);
void ShowCourseBest_m1x1(void);
void ShowCourseRecord_m1x1(void);
void ShowDigitDifference_m1x1(void);
void ShowEntryCode_m1x1(void);
void ShowItemPoints_m1x1(void);
void ShowJumpCountLong_m1x1(void);
void ShowJumpCount_m1x1(void);
void ShowJumpResult_m1x1(void);
void ShowLastDistance_m1x1(void);
void ShowResultsBanner_m1x1(void);
void ShowStadium1Row_m1x1(void);
void SinCos_m1x1(void);
void SkyFadeStep_m1x1(void);
void SlotRiderShadow_m1x1(void);
void SlowTileCheck_m1x1(void);
void SortAndDrawRiders_m1x1(void);
void SortRidersByPosition_m1x1(void);
void SpawnDustPuff_m1x1(void);
void SpawnExhaust_m1x1(void);
void SpawnObject_m1x1(void);
void SpawnPlayerSpark_m1x1(void);
void SpawnRivals_m1x1(void);
void SpcUploadB_m1x1(void);
void SpeedGauge_m1x1(void);
void SpeedReadout_m1x1(void);
void SpinAllWheels_m1x1(void);
void SpinWheels8_m1x1(void);
void SpinWheels_m1x1(void);
void StadiumBonusBanner_m1x1(void);
void StartBonusCount_m1x1(void);
void StartBonusCourse_m1x1(void);
void StartBonusMode_m1x1(void);
void StartBonusResult_m1x1(void);
void StartBonusTry_m1x1(void);
void StartCrash_m1x1(void);
void StartCredits_m1x1(void);
void StartDemoRace_m1x1(void);
void StartEndScreen_m1x1(void);
void StartIntroMode_m1x1(void);
void StartJumpBase30_m1x1(void);
void StartJumpMode_m1x1(void);
void StartMode01_m1x1(void);
void StartModeWithSound_m1x1(void);
void StartPlayerSelect_m1x1(void);
void StartPracticeMode_m1x1(void);
void StartRaceBoard_m1x1(void);
void StartRandomCourse_m1x1(void);
void StartSession_m1x1(void);
void StartWheelie_m1x1(void);
void SteerObjectStep_m1x1(void);
void StepAnimTimer_m1x1(void);
void StepColourCycle_m1x1(void);
void StepFade_m1x1(void);
void StepFallingSprite_m1x1(void);
void StepJumpBanner_m1x1(void);
void StepLfsr_m1x1(void);
void StepObjectScript_m1x1(void);
void StepObjectX8_m1x1(void);
void StepObjectX_m1x1(void);
void StepSpark_m1x1(void);
void StoreEntryNumber_m1x1(void);
void ThrottleAll_m1x1(void);
void Tick08CC_m1x1(void);
void TickRaceClock_m1x1(void);
void TickTimers_m1x1(void);
void TitlePushStart_m1x1(void);
void TrackRiderPace_m1x1(void);
void TurboMeterJmp_m1x1(void);
void TurboMeter_m1x1(void);
void UnpackToLowRam_m1x1(void);
void UnpackToWram_m1x1(void);
void UpdateLanes_m1x1(void);
void UpdateObjects_m1x1(void);
void UpdateRiderDistances_m1x1(void);
void UseArrowPiece_m1x1(void);
void VelocityDirection_m1x1(void);
void VramQueueTiles20_m1x1(void);
void VramQueueTilesLong_m1x1(void);
void VramQueueTiles_m1x1(void);
void WalkToStartLane_m1x1(void);
void WinnerFigure_m1x1(void);
void VramQueueTilesFrom(u32 list);
static void end_record(void);
static void begin_record(u16 off, u8 words);
static void queue20(u16 vram, u16 list);
void SpcUploadFrom(u32 stream);
static void upload_from_dp(void);
static void draw_portrait(int i, u8 cursor, int st);
static u8 portrait_angle(int i);
static void portrait_xy(int i, u8 *x, u8 *y);
static void queue_sound_body(void);
static void random_body(void);
static u8 split_tens(u8 v, u16 tens);
static void crash_sound(u8 r, u16 ret);
static void sound80(u8 cmd, u16 ret);
static void read_pad(u16 reg_lo, u8 held_lo, u8 press_lo, u8 prev_lo,
                     u8 held_hi, u8 press_hi, u8 prev_hi);
static void apply_speed(u16 speed, u16 frac, u16 pos);
static void fill_queue_row(u8 last, u16 tile);
static void object_advance(u8 delay, u8 next);
static void object_advance_or(u8 delay, u8 next, u8 wait);
static void hundreds(void);
static void scroll_by_speed16(void);
static void stream_step(void);
static void object_advance_random(u8 delay, u8 next, u8 wait, u16 ret);
static void object_cycle(u8 zero, u16 delays, u16 frames, int tay_first);
static void object_repeat(u8 mask, u8 state, u16 delays, u16 frames, u16 ret);
static void object_frame(u16 delays, u16 frames);
static void object_rest(u8 state, u16 ret);
static void random_rest(u16 ret);
static int object_ready(void);
static void spin_wheels(void);
static void draw_countdown_sign(u16 table);
static void or_digits(u8 v, u16 hi, u16 lo);
static void build_time(u16 m, u16 s, u16 f, int last_add);
static void fade_field(u16 m, u16 target, u16 u);
static void fade_to(u16 m, u16 targets, u16 u);
static void bonus_result_body(void);
static void queue_bonus_tiles(u16 t0, u16 t1);
static void queue_item_tiles(u8 dp, u16 bases, u16 t0, u16 t1);
static void pace_target(void);
static void crash_from(void);
static void crash_tail(void);
static void crash_stop(void);
static void input_and_a(void);
static void input_or_y(void);
static void start_flags(void);
static void crash_loop(int enter_turn);
static void mul16(u8 p, u8 q, u8 r, const u16 back[4]);
static void mul8(u8 a, u8 b, u16 back);
static void mul16b(void);
static void queue_rows80(u16 vram, u16 src, u16 size, u16 back);
static void scale_jump_tail(void);
static void unpack_stream(void);
static int jump_result_wave(u16 back);
static void acc_mul(u8 value, u16 mult, u16 call);
static void demo_exit(void);
static void overheat_dust(void);
static void lane_piece(void);
static void player_column(u16 base);
static void player_crash_tail(u16 base);
static void player_stop(void);
static void steer_lane(int up, int from_ahead);
static void course_column(u16 lo_table, u16 hi_table, u16 back);
static void steer_by_e6(void);
static void player_halt(void);
static u32 piece_addr(void);
static void player_piece_type(u16 back);
static void course_map_slot(void);
static void height_below(void);
static void landing_crash(void);
static void frame_sequence(u8 count, u16 times, u8 spawn, u16 dy,
                           u16 frames, u16 back);
static void player_crash_step(int b);
static void results_score_tail(void);
static void lean_toward(void);
static void lean_by_pad(void);
static void course_records_step(void);
static u8 course_list4(u32 dp);
static u8 script_byte4(void);
static void lean_and_bands4(u16 back1, u16 back2);
static void vram_fill_half4(u16 vram, int wide_stz, u16 bytes);
static void bsx_exit4(u32 site);
static void restart_minute4(u16 ret_addr, u8 a);
static void course_start_riders4(int select);
static void table_jump4(u32 table, u8 v, u32 site);
static void object_state_jump4(u32 table, u32 site);
static void set_jump_state4(void);
static void demo_next_entry4(void);
static void send_pitch4(void);
static void rti4(void);
static void nmi_sound_channel4(u8 n);
R816_EXPORT void MainLoopFrame4(void);
R816_EXPORT void ResetBootAll(void);
R816_EXPORT void ResetBoot4b(void);
static void wait_apu_idle4(void);
R816_EXPORT void ResetBoot4aSpc(void);
R816_EXPORT void ResetBoot4a(void);

/* The queue body, reading the list from a full 24-bit address (the port's
 * added characters keep their names outside bank $80). */
void VramQueueTilesFrom(u32 list)
{
    wr8(DP(0x0E), 0);
    push8((u8)X);
    push8((u8)Y);
    u8 size = rd8(list);
    u8 words = size & 0x0F;
    wr8(DP(0x05), words);
    wr8(DP(0x08), (u8)(words << 1));
    wr8(DP(0x09), 0);
    wr8(DP(0x04), size >> 4);
    u16 y = 0;
    do {
        u16 off = rd16(LONG(VQ_END));
        begin_record(off, rd8(DP(0x05)));
        u16 x = off;
        do {
            y++;
            u16 tile = (u16)((rd16((list + y) & 0xFFFFFF) & 0x00FF) | rd16(DP(0x0E)));
            wr16(q(x, 0x0C), tile);
            x += 2;
            wr8(DP(0x06), (u8)(rd8(DP(0x06)) - 1));
        } while (rd8(DP(0x06)) != 0);
        end_record();
        wr8(DP(0x04), (u8)(rd8(DP(0x04)) - 1));
        X = x; Y = y;
    } while (rd8(DP(0x04)) != 0);
    sep(0x10);
    Y = pull8();
    ldx8(pull8());
    ret();
}
/* Close a record: link it, and step the VRAM address one tilemap row. */
static void end_record(void)
{
    u16 head = rd16(LONG(VQ_END));
    u16 next = (u16)(head + 0x0C + rd16(DP(0x08)));
    wr16(LONG(VQ_END), next);
    wr16(q(head, 0x0A), next);
    A = next;                                       /* high byte stays in B */
    SET_A8(0x20); fc = 0;
    adc8(rd8(DP(0x00))); wr8(DP(0x00), (u8)A);
    SET_A8(0x00);
    adc8(rd8(DP(0x01))); wr8(DP(0x01), (u8)A);
}
/* Record header at queue offset `off`: VMAIN $80, DMAP $01, BBAD $18, source
 * $7F:(off + $0C), VRAM address $00/$01, byte count words * 2. */
static void begin_record(u16 off, u8 words)
{
    wr8(q(off, 2), 0x80);
    wr8(q(off, 3), 0x01);
    wr8(q(off, 4), 0x18);
    wr16(q(off, 5), (u16)(off + 0x0C));
    wr8(q(off, 7), 0x7F);
    wr8(q(off, 1), rd8(DP(0x01)));
    wr8(q(off, 0), rd8(DP(0x00)));
    wr8(DP(0x06), words);
    wr8(q(off, 8), (u8)(words << 1));
    wr8(q(off, 9), 0x00);
}
static void queue20(u16 vram, u16 list)
{
    rep(0x20);
    lda16(vram); wr16(DP(0x00), vram);
    lda16(list); wr16(DP(0x02), list);
    sep(0x20);
}
/* The same upload from a given stream address (an added character's). */
void SpcUploadFrom(u32 stream)
{
    lda8(0xFF);
    wr8(ABS(APUIO0), 0xFF);
    wr8(DP(0x00), (u8)stream);
    wr8(DP(0x01), (u8)(stream >> 8));
    wr8(DP(0x02), (u8)(stream >> 16));
    upload_from_dp();
}
/* Upload the stream at the long pointer $00-$02 with the driver's transfer
 * handshake (APUIO0-2), then clear the ports. */
static void upload_from_dp(void)
{
    wr8(ABS(0x1978), 0);
    push8(DB);
    lda8(0x00);
    push8(0x00);
    DB = pull8(); nz8(DB);
    push8(getP());
    rep(0x30);
    Y = 0;
    lda16(0xBBAA);
    do {
        WAIT_TICK();
        cmp16(A, rd16(ABS(APUIO0)));
    } while (!fz);
    sep(0x20);
    lda8(0xCC);
    for (;;) {
        push8((u8)A);
        rep(0x20);
        u32 src = ILDP(0x00);
        u16 len = rd16((src + Y) & 0xFFFFFF); Y += 2;
        X = len;
        u16 dest = rd16((src + Y) & 0xFFFFFF); Y += 2;
        wr16(ABS(APUIO2), dest);
        sep(0x20);
        cmp16(X, 0x0001);
        lda8(0x00);
        lda8(rol8((u8)A));
        wr8(ABS(APUIO1), (u8)A);
        adc8(0x7F);
        lda8(pull8());
        wr8(ABS(APUIO0), (u8)A);
        while (rd8(ABS(APUIO0)) != (u8)A)
            WAIT_TICK();
        if (!fv)
            break;
        /* one block of X bytes */
        lda8(rd8((ILDP(0x00) + Y) & 0xFFFFFF)); Y++;
        A = (u16)((A << 8) | (A >> 8));
        lda8(0x00);
        for (;;) {
            rep(0x20);
            wr16(ABS(APUIO0), A);
            sep(0x20);
            X--;
            if (X == 0) break;
            A = (u16)((A << 8) | (A >> 8));
            lda8(rd8((ILDP(0x00) + Y) & 0xFFFFFF)); Y++;
            if (Y == 0x8000) { Y = 0; wr8(DP(0x02), inc8(rd8(DP(0x02)))); }
            A = (u16)((A << 8) | (A >> 8));
            while (rd8(ABS(APUIO0)) != (u8)A)
                WAIT_TICK();
            lda8(inc8((u8)A));
        }
        do {                                    /* CMP: carry set on the match */
            WAIT_TICK();
            cmp8((u8)A, rd8(ABS(APUIO0)));
        } while (!fz);
        do { adc8(0x03); } while (fz);
    }
    wr8(ABS(APUIO0), 0);
    wr8(ABS(APUIO1), 0);
    wr8(ABS(APUIO2), 0);
    wr8(ABS(APUIO3), 0);
    setP(pull8());
    DB = pull8(); nz8(DB);
}
/* One portrait: a 3x3 block of 16x16 sprites at OAM offset oam. */
static void draw_portrait(int i, u8 cursor, int st)
{
    u8 px, py;
    portrait_xy(i, &px, &py);
    wr8(DP(0x04), px);
    lda8(py); fc = 1; sbc8(0x10);
    wr8(DP(0x00), (u8)A);
    wr8(DP(0x10), (u8)i);
    u8 ii = rd8(DP(0x10));                          /* the original indexes with $10 */
    lda8((u8)(cursor * 5 + ii));
    u8 oam = roster_select_oam(cursor, ii);
    Y = oam; nz8(oam);
    push8((u8)i);
    lda8(rd8(DP(0x04)));
    for (int c = 0; c < 3; c++) {
        wr8(ABS(OAM_BUF + Y + c * 4), (u8)A);
        wr8(ABS(OAM_BUF + 0x0C + Y + c * 4), (u8)A);
        wr8(ABS(OAM_BUF + 0x18 + Y + c * 4), (u8)A);
        if (c < 2) { fc = 0; adc8(0x10); }
    }
    lda8(rd8(DP(0x00)));
    for (int r = 0; r < 3; r++) {
        wr8(ABS(OAM_BUF + 0x01 + Y + r * 12), (u8)A);
        wr8(ABS(OAM_BUF + 0x05 + Y + r * 12), (u8)A);
        wr8(ABS(OAM_BUF + 0x09 + Y + r * 12), (u8)A);
        if (r < 2) { fc = 0; adc8(0x10); }
    }
    u8 tile, nt, selpal;
    roster_select_portrait(st, i, &tile, &nt, &selpal);
    lda8(tile);
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            wr8(ABS(OAM_BUF + 0x02 + Y + r * 12 + c * 4), (u8)A);
            if (c < 2) { lda8(inc8((u8)A)); lda8(inc8((u8)A)); }
        }
        if (r < 2) { fc = 0; adc8(0x1C); }
    }
    u8 attr;
    lda8(rd8(DP(P1_HELD_HI)));
    and8(0x03);
    int dim = !fz;
    if (!dim) {
        lda8(portrait_angle(i));
        dim = !fz;
    }
    if (dim) {
        lda8(nt);
        ora8(0x28);
        attr = (u8)A;
    } else {
        X = (u8)i;
        push8(0x80); push16(0x8C89); SelectName_m1x1();
        lda8(selpal);
        ora8(nt);
        attr = (u8)A;
    }
    for (int k = 0; k < 9; k++)
        wr8(ABS(OAM_BUF + 0x03 + Y + k * 4), attr);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02);
    for (int k = 0; k < 9; k++)
        wr8(ABS(OAM_SIZE_BUF + Y + k), 0x02);
    X = pull8();
}
/* Angle of portrait i (0-4: their own byte; added ones follow portrait 0). */
static u8 portrait_angle(int i)
{
    if (i < 5 && !roster_select_general())
        return rd8(ABS(0x1554 + i));
    return (u8)(rd8(ABS(0x1554)) + roster_select_ring_offset(i) - roster_select_ring_offset(0));
}
/* Screen position of an added portrait (the original keeps 0-4 in RAM). */
static void portrait_xy(int i, u8 *x, u8 *y)
{
    if (i < 5 && !roster_select_general()) {
        *x = rd8(ABS(0x155A + i));
        *y = rd8(ABS(0x155F + i));
        return;
    }
    u16 sx = X, sy = Y;
    wr8(DP(0x08), 0x40);
    X = portrait_angle(i);
    push8(0x81); push16(0xA257); SinCos_m1x1();
    *x = (u8)(0x68 + rd8(DP(0x04)));
    *y = (u8)(0x50 + rd8(DP(0x00)));
    X = sx; Y = sy;
}
/* $82:862C QueueSoundB (RTL; entered with M=0 or M=1): append sound command
 * A to the ring at $1400 (index $65, 64 entries); $71 = last command. */
static void queue_sound_body(void)
{
    push8(getP());
    rep(0x10);
    push16(X);
    sep(0x30);
    wr8(ABS(SND_LAST), (u8)A);
    X = rd8(ABS(SND_WRITE));
    wr8(ABS(SND_RING + X), (u8)A);
    X = (u8)(X + 1);
    SET_A8((u8)X);
    and8(0x3F);
    wr8(ABS(SND_WRITE), (u8)A);
    rep(0x10);
    X = pull16();
    setP(pull8());
    retl();
}
/* $81:A304 RandomB (RTL), as Stadium 1's: mixes the latched H/V counters,
 * the frame counter $63/$64 and the broadcast clock into A. */
static void random_body(void)
{
    lda8(rd8(ABS(SLHV)));
    lda8(rd8(ABS(STAT78)));
    adc8(rd8(ABS(OPHCT)));
    adc8(rd8(ABS(OPHCT)));
    adc8(rd8(ABS(OPVCT)));
    adc8(rd8(ABS(OPVCT)));
    eor8(rd8(DP(FRAME_COUNT)));
    adc8(rd8(DP(0x64)));
    adc8(rd8(LONG(BCAST_MINUTE)));
    adc8(rd8(LONG(BCAST_FRAME)));
    adc8(rd8(LONG(BCAST_FRAME_HI)));
    retl();
}
/* Two-digit split by repeated subtraction into the tens byte at `tens`
 * (cleared first); returns the units. */
static u8 split_tens(u8 v, u16 tens)
{
    wr8(ABS(tens), 0);
    lda8(v);
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        wr8(ABS(tens), inc8(rd8(ABS(tens))));
        fc = 1;
        sbc8(0x0A);
    }
    return (u8)A;
}
/* Crash sound for rider r: $24, or $29 ($24 + 5) for a rival. */
static void crash_sound(u8 r, u16 ret)
{
    lda8(r);
    if (!fz) lda8(0x05);
    fc = 0;
    adc8(0x24);
    push8(0x80); push16(ret); QueueSoundB_m1x1();
}
/* JSL QueueSoundB from bank $80 with the return address `ret`. */
static void sound80(u8 cmd, u16 ret)
{
    lda8(cmd);
    push8(0x80); push16(ret); QueueSoundB_m1x1();
}
/* One pad: ignore anything that is not a standard controller (low nibble of
 * JOYxL set), then update held / newly pressed / previous for both halves. */
static void read_pad(u16 reg_lo, u8 held_lo, u8 press_lo, u8 prev_lo,
                     u8 held_hi, u8 press_hi, u8 prev_hi)
{
    wr8(DP(0x00), 0);
    wr8(DP(0x01), 0);
    u8 lo = rd8(ABS(reg_lo));
    if ((lo & 0x0F) == 0) {
        wr8(DP(0x00), lo);
        wr8(DP(0x01), rd8(ABS(reg_lo + 1)));
    }
    lo = rd8(DP(0x00));
    wr8(DP(held_lo), lo);
    /* EOR prev / AND held: the original re-reads the held byte from memory */
    wr8(DP(press_lo), (u8)((lo ^ rd8(DP(prev_lo))) & rd8(DP(held_lo))));
    wr8(DP(prev_lo), lo);
    u8 hi = rd8(DP(0x01));
    wr8(DP(held_hi), hi);
    u8 pressed = (u8)((hi ^ rd8(DP(prev_hi))) & rd8(DP(held_hi)));
    wr8(DP(press_hi), pressed);
    wr8(DP(prev_hi), hi);
    SET_A8(pressed);
    Y = hi;
}
/* Add the signed 4.4 speed at speed,X to an object coordinate: the low nibble
 * (x16) into the fraction frac,X, the high nibble, sign-extended, plus the
 * carry into the 16-bit position pos/pos+1,X. */
static void apply_speed(u16 speed, u16 frac, u16 pos)
{
    u8 v = rd8(ABS(speed + X));
    lda8(v);
    lda8(asl8(asl8(asl8(asl8(v)))));
    fc = 0;
    adc8(rd8(ABS(frac + X)));
    wr8(ABS(frac + X), (u8)A);
    push8(getP());                                 /* keep the fraction's carry */
    lda8(v);
    lda8(lsr8(lsr8(lsr8(lsr8(v)))));
    cmp8((u8)A, 0x08);
    ldy8(0x00);
    if (fc) {                                      /* negative speed: sign-extend */
        ora8(0xF0);
        Y = dec8((u8)Y);
    }
    setP(pull8());
    adc8(rd8(ABS(pos + X)));
    wr8(ABS(pos + X), (u8)A);
    lda8((u8)Y);
    adc8(rd8(ABS(pos + 1 + X)));
    wr8(ABS(pos + 1 + X), (u8)A);
}
/* Fill words $7F:000E..$7F:000C+last with a tile word (a blank row in the
 * VRAM queue buffer). */
static void fill_queue_row(u8 last, u16 tile)
{
    ldx8(last);
    rep(0x20);
    lda16(tile);
    do {
        wr16(LONGX(VQ_DATA), tile);
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fz);
    sep(0x20);
}
static void object_advance(u8 delay, u8 next) { object_advance_or(delay, next, 0); }
/* Object $03A3: when its counter $037F is zero, set the delay $036F, clear
 * the timer $031F, step the state $030F and set $034F to `next`; otherwise
 * set $034F to `wait` (STZ when it is zero). */
static void object_advance_or(u8 delay, u8 next, u8 wait)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(ABSX(OBJ_COUNTER)));
    if (!fz) {
        if (wait) lda8(wait);
        wr8(ABSX(OBJ_FRAME), wait);
        return;
    }
    lda8(delay); wr8(ABSX(OBJ_DELAY), delay);
    wr8(ABSX(OBJ_STEP), 0);
    wr8(ABSX(OBJ_STATE), inc8(rd8(ABSX(OBJ_STATE))));
    lda8(next); wr8(ABSX(OBJ_FRAME), next);
}
/* $81:CA05 Hundreds (RTS, entered with M either way): Y = the word $1569
 * divided by 200 (at least 1); A keeps the remainder. */
static void hundreds(void)
{
    rep(0x20);
    ldy8(0x00);
    lda16(rd16(ABS(JUMP_COUNT)));
    for (;;) {
        cmp16(A, 0x00C8);
        if (!fc) break;
        Y = inc8((u8)Y);
        fc = 1;
        sbc16(0x00C8);
        WAIT_TICK();
    }
    sep(0x20);
    cmp8((u8)Y, 0x00);
    if (fz) Y = inc8((u8)Y);
    ret();
}
/* $80:DA0E, $80:ECD8 ScrollBySpeed16 (RTS): fraction $0994 += speed $0186
 * * 16 (the carry out of the shifts goes in), carry into $93; when $93
 * wraps flip the half $91. */
static void scroll_by_speed16(void)
{
    lda8(rd8(ABS(RIDER_STEP)));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    adc8(rd8(ABS(SCROLL_FRAC)));
    wr8(ABS(SCROLL_FRAC), (u8)A);
    lda8(rd8(DP(0x93)));
    adc8(0x00);
    wr8(DP(0x93), (u8)A);
    if (fc) {
        lda8(rd8(DP(0x91)));
        eor8(0x01);
        wr8(DP(0x91), (u8)A);
    }
    ret();
}
/* $82:8CD4 ReadStreamWord (M=1, X=0, RTS): two stream bytes, the first in
 * the high byte of A (same pointer rules as ReadStreamByte). */
static void stream_step(void)
{
    u32 p = rd16(DP(0x08)) | ((u32)rd8(DP(0x0A)) << 16);
    lda8(rd8(p & 0xFFFFFF));
    X = inc16(X);
    if (fz) {
        ldx16(0x8000);
        wr8(DP(0x0A), inc8(rd8(DP(0x0A))));
    }
    wr16(DP(0x08), X);
}
/* When the counter is zero: delay `delay`, a random count 1-4, restart the
 * step, next state, frame `next`. Otherwise the frame is `wait` (STZ when
 * zero). `ret` is the JSL return address minus one. */
static void object_advance_random(u8 delay, u8 next, u8 wait, u16 ret)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(ABSX(OBJ_COUNTER)));
    if (!fz) {
        if (wait) lda8(wait);
        wr8(ABSX(OBJ_FRAME), wait);
        return;
    }
    lda8(delay); wr8(ABSX(OBJ_DELAY), delay);
    push8(0x81); push16(ret); RandomB_m1x1();
    and8(0x03);
    lda8(inc8((u8)A));
    wr8(ABSX(OBJ_RANDOM), (u8)A);
    wr8(ABSX(OBJ_STEP), 0);
    wr8(ABSX(OBJ_STATE), inc8(rd8(ABSX(OBJ_STATE))));
    lda8(next); wr8(ABSX(OBJ_FRAME), next);
}
/* When the delay has run out, step $031F through 1-3 (0 becomes `zero`)
 * and load the delay and frame from the tables at `delays`/`frames`.
 * `tay_first` follows $81:BFCD, which moves the step into Y before the
 * zero test. */
static void object_cycle(u8 zero, u16 delays, u16 frames, int tay_first)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(ABSX(OBJ_DELAY)));
    if (!fz) return;
    lda8(rd8(ABSX(OBJ_STEP)));
    lda8(inc8((u8)A));
    and8(0x03);
    wr8(ABSX(OBJ_STEP), (u8)A);
    if (tay_first) { Y = (u8)A; nz8((u8)Y); }
    if (fz) {
        if (tay_first) lda8(zero); else lda8(inc8((u8)A));
        wr8(ABSX(OBJ_STEP), (u8)A);
        if (tay_first) { Y = (u8)A; nz8((u8)Y); }
    }
    if (!tay_first) { Y = (u8)A; nz8((u8)Y); }
    lda8(rd8(ABSY(delays))); wr8(ABSX(OBJ_DELAY), (u8)A);
    lda8(rd8(ABSY(frames))); wr8(ABSX(OBJ_FRAME), (u8)A);
}
/* Flip the step between 0 and 1 (`mask` 1), or count it 0-3 (`mask` 3).
 * Each time it comes back to 0 count $032F down; at zero rest in `state`. */
static void object_repeat(u8 mask, u8 state, u16 delays, u16 frames, u16 ret)
{
    if (!object_ready()) return;
    lda8(rd8(ABSX(OBJ_STEP)));
    if (mask == 1) eor8(0x01);
    else { lda8(inc8((u8)A)); and8(mask); }
    wr8(ABSX(OBJ_STEP), (u8)A);
    Y = (u8)A; nz8((u8)Y);
    if (fz) {
        u8 n = dec8(rd8(ABSX(OBJ_RANDOM)));
        wr8(ABSX(OBJ_RANDOM), n);
        if (fz) { object_rest(state, ret); return; }
    }
    object_frame(delays, frames);
}
/* Load the delay and frame for step Y. */
static void object_frame(u16 delays, u16 frames)
{
    lda8(rd8(ABSY(delays))); wr8(ABSX(OBJ_DELAY), (u8)A);
    lda8(rd8(ABSY(frames))); wr8(ABSX(OBJ_FRAME), (u8)A);
}
/* Put the object to rest: state `state` (STZ when zero) and a random pause
 * $037F of $40-$7F frames. */
static void object_rest(u8 state, u16 ret)
{
    if (state) lda8(state);
    wr8(ABSX(OBJ_STATE), state);
    random_rest(ret);
    wr8(ABSX(OBJ_COUNTER), (u8)A);
}
/* A = RandomB() & $3F + $40 (`ret`: JSL return address minus one). */
static void random_rest(u16 ret)
{
    push8(0x81); push16(ret); RandomB_m1x1();
    and8(0x3F);
    fc = 0;
    adc8(0x40);
}
/* Selects the object; returns 0 while its delay runs. */
static int object_ready(void)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(ABSX(OBJ_DELAY)));
    return fz;
}
/* Wheel animation for riders X..0: while moving, count $018E down by the
 * speed $0186 and on each wrap (+8) step the wheel frame $0266 (0-63);
 * clear $0246. */
static void spin_wheels(void)
{
    do {
        lda8(rd8(ABSX(RIDER_STEP)));
        if (!fz) {
            lda8(rd8(ABSX(RIDER_WHEEL)));
            fc = 1;
            sbc8(rd8(ABSX(RIDER_STEP)));
            if (!fz && !fn) {
                wr8(ABSX(RIDER_WHEEL), (u8)A);
            } else {
                fc = 0;
                adc8(0x08);
                wr8(ABSX(RIDER_WHEEL), (u8)A);
                wr8(ABSX(RIDER_COLUMN), inc8(rd8(ABSX(RIDER_COLUMN))));
                lda8(rd8(ABSX(RIDER_COLUMN)));
                and8(0x3F);
                wr8(ABSX(RIDER_COLUMN), (u8)A);
            }
        }
        lda8(0x00);
        wr8(ABSX(RIDER_SLOW_AT), 0x00);
        X = dec8((u8)X);
    } while (!fn);
}
/* Countdown sign: while $C3 (1-4) is on, copy its 8-byte sprite set from
 * `table` + ($C3 - 1) * 8 to OAM buffer $16A0-$16A7 (both large). At 4
 * it stays up until the timer $8A runs out. */
static void draw_countdown_sign(u16 table)
{
    lda8(rd8(DP(COUNTDOWN)));
    if (fz) return;
    cmp8((u8)A, 0x04);
    if (fz) {
        lda8(rd8(DP(TIMER_A)));
        if (fz) return;
        wr8(DP(TIMER_A), dec8(rd8(DP(TIMER_A))));
    }
    push8((u8)X);
    lda8(rd8(DP(COUNTDOWN)));
    lda8(dec8((u8)A));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    ldy8(0xA0);
    rep(0x20);
    do {
        lda16(rd16(ABSX(table)));
        wr16(ABSY(OAM_BUF), A);
        X = inc8((u8)X); X = inc8((u8)X);
        Y = inc8((u8)Y); Y = inc8((u8)Y);
        cmp8((u8)Y, 0xA8);
    } while (!fc);
    ldy8(0x28);
    lda16(0x0202);
    wr16(ABSY(OAM_SIZE_BUF), 0x0202);
    sep(0x20);
    X = pull8(); nz8((u8)X);
}
/* Two decimal digits of `v`: the units ORed into `lo`, the tens into
 * `hi`. */
static void or_digits(u8 v, u16 hi, u16 lo)
{
    ldy8(0x00);
    lda8(v);
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
        WAIT_TICK();
    }
    ora8(rd8(ABS(lo)));
    wr8(ABS(lo), (u8)A);
    lda8((u8)Y);
    ora8(rd8(ABS(hi)));
    wr8(ABS(hi), (u8)A);
}
/* Time display "M'SS''FF" at $03CF (size $27) from minutes `m`, seconds
 * `s` and frames `f`: digits via the tables $80:C470/$80:C4AC, separator
 * tiles $47/$48 and the lower row (+$10). `last_add` picks the final
 * operation for the last lower tile (ORA or ADC $10). */
static void build_time(u16 m, u16 s, u16 f, int last_add)
{
    rep(0x20);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    X = rd8(ABS(f)); nz8((u8)X);
    lda16(rd16(ABSX(0xC479)));
    lda16(asl16(A));
    X = (u8)A; nz8((u8)X);
    lda16(rd16(ABSX(0xC4B5))); wr16(ABS(DIGIT_ROW + 0x05), A);
    lda16(rd16(ABS(s)));
    lda16(asl16(A));
    X = (u8)A; nz8((u8)X);
    lda16(rd16(ABSX(0xC4B5))); wr16(ABS(DIGIT_ROW + 0x02), A);
    sep(0x20);
    lda8(rd8(ABS(m))); wr8(ABS(DIGIT_ROW), (u8)A);
    lda8(0x27); wr8(ABS(DIGIT_BUF), 0x27);
    rep(0x20);
    lda16(rd16(ABS(DIGIT_ROW)));
    and16(0x00FF);
    ora16(0x4700);
    wr16(ABS(DIGIT_ROW), A);
    ora16(0xFF10);
    wr16(ABS(DIGIT_ROW + 0x07), A);
    lda16(rd16(ABS(DIGIT_ROW + 0x02)));
    ora16(0x1010);
    wr16(ABS(DIGIT_ROW + 0x09), A);
    lda16(rd16(ABS(DIGIT_ROW + 0x04)));
    and16(0xFF00);
    ora16(0x0048);
    wr16(ABS(DIGIT_ROW + 0x04), A);
    ora16(0x10FF);
    wr16(ABS(DIGIT_ROW + 0x0B), A);
    sep(0x20);
    lda8(rd8(ABS(DIGIT_ROW + 0x06)));
    if (last_add) { fc = 0; adc8(0x10); } else ora8(0x10);
    wr8(ABS(DIGIT_ROW + 0x0D), (u8)A);
}
/* Step one 5-bit colour field of the word `v` (mask `m`, unit `u`)
 * towards `target`. */
static void fade_field(u16 m, u16 target, u16 u)
{
    lda16(rd16(ABSX(PALETTE_BUF + 0x38)));
    and16(m);
    cmp16(A, target);
    if (fz) return;
    int down = fc;
    lda16(rd16(ABSX(PALETTE_BUF + 0x38)));
    if (down) { fc = 1; sbc16(u); }
    else { fc = 0; adc16(u); }
    wr16(ABSX(PALETTE_BUF + 0x38), A);
}
/* Step one colour field of the word $1C00,X towards the target word at
 * `targets`,X (mask `m`, unit `u`). */
static void fade_to(u16 m, u16 targets, u16 u)
{
    lda16(rd16(ABSX(PALETTE_BUF)));
    and16(m);
    cmp16(A, rd16(ABSX(targets)));
    if (fz) return;
    int down = fc;
    lda16(rd16(ABSX(PALETTE_BUF)));
    if (down) { fc = 1; sbc16(u); }
    else { fc = 0; adc16(u); }
    wr16(ABSX(PALETTE_BUF), A);
}
/* $80:DE65: the result itself (also entered from $80:DDEF). */
static void bonus_result_body(void)
{
    lda8(rd8(ABS(BONUS_RESULT)));
    cmp8((u8)A, rd8(ABS(0x1E88)));
    if (fc) wr8(ABS(0x1E88), (u8)A);
    lda8(rd8(ABS(0x1E87)));
    int show = 1;
    if (!fn) { cmp8((u8)A, 0x0A); show = !fc; }
    if (show) {
        rep(0x20);
        lda16(0x1950); wr16(DP(0x00), 0x1950);
        lda16(0xDF26); wr16(DP(0x02), 0xDF26);
        sep(0x20);
        push16(0xDE95); VramQueueTiles20_m1x1();
        rep(0x20);
        lda16(0x1956); wr16(DP(0x00), 0x1956);
        lda16(0x03CF); wr16(DP(0x02), 0x03CF);
        sep(0x20);
        lda8(0x00);
        wr8(ABS(DIGIT_ROW), 0x00);
        wr8(ABS(DIGIT_ROW), inc8(rd8(ABS(DIGIT_ROW))));
        for (u16 a = 0x03D1; a <= 0x03D4; a++) wr8(ABS(a), 0x00);
        lda8(0x10);
        wr8(ABS(DIGIT_ROW + 0x05), 0x10);
        wr8(ABS(DIGIT_ROW + 0x05), inc8(rd8(ABS(DIGIT_ROW + 0x05))));
        for (u16 a = 0x03D6; a <= 0x03D9; a++) wr8(ABS(a), 0x10);
        lda8(rd8(ABS(BONUS_RESULT)));
        cmp8((u8)A, rd8(ABS(0x09C9)));
        if (!fz) {
            lda8(0xFF);
            wr8(ABS(DIGIT_ROW), 0xFF);
            wr8(ABS(DIGIT_ROW + 0x05), 0xFF);
            ldy8(0x00);
            lda8(rd8(ABS(BONUS_RESULT)));
            for (;;) {
                cmp8((u8)A, 0x0A);
                if (!fc) break;
                sbc8(0x0A);
                Y = inc8((u8)Y);
                WAIT_TICK();
            }
            wr8(ABS(DIGIT_ROW + 0x02), (u8)A);
            ora8(0x10); wr8(ABS(DIGIT_ROW + 0x07), (u8)A);
            lda8((u8)Y);
            wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
            ora8(0x10); wr8(ABS(DIGIT_ROW + 0x06), (u8)A);
            lda8(rd8(ABS(DIGIT_ROW + 0x01)));
            if (fz) {
                lda8(0xFF);
                wr8(ABS(DIGIT_ROW + 0x01), 0xFF); wr8(ABS(DIGIT_ROW + 0x06), 0xFF);
                lda8(rd8(ABS(DIGIT_ROW + 0x02)));
                if (fz) {
                    lda8(0xFF);
                    wr8(ABS(DIGIT_ROW + 0x02), 0xFF); wr8(ABS(DIGIT_ROW + 0x03), 0xFF);
                    wr8(ABS(DIGIT_ROW + 0x07), 0xFF); wr8(ABS(DIGIT_ROW + 0x08), 0xFF);
                }
            }
        }
        lda8(0x25); wr8(ABS(DIGIT_BUF), 0x25);
        push16(0xDF22); VramQueueTiles20_m1x1();
    }
    rep(0x20);
    lda16(0x0180); wr16(DP(MODE_TIMER), 0x0180);
    sep(0x20);
    wr8(DP(MODE_STEP), inc8(rd8(DP(MODE_STEP))));
    ret();
}
/* Queue the 2-tile pair `t0`/`t1` (tables by $B7) for the track column
 * $0266 - $10 into the BG map at `bases` ($80:A84F by 2 * $B7). */
static void queue_bonus_tiles(u16 t0, u16 t1)
{
    rep(0x20);
    lda16(rd16(ABS(RIDER_COLUMN)));
    fc = 1; sbc16(rd16(DP(0x10)));
    and16(0x0020);
    for (int i = 0; i < 5; i++) lda16(asl16(A));
    wr16(DP(0x00), A);
    lda16(rd16(ABS(RIDER_COLUMN)));
    fc = 1; sbc16(rd16(DP(0x10)));
    and16(0x001F);
    fc = 0; adc16(rd16(DP(0x00)));
    fc = 0; adc16(rd16(ABSX(0xA858)));
    wr16(DP(0x00), A);
    lda16(0x0018); wr16(DP(0x02), 0x0018);
    sep(0x20);
    X = rd8(DP(CLOCK_MINUTE)); nz8((u8)X);
    lda8(0x21); wr8(DP(0x18), 0x21);
    lda8(rd8(ABSX(t0))); wr8(DP(0x19), (u8)A);
    lda8(rd8(ABSX(t1))); wr8(DP(0x1A), (u8)A);
    lda8(0x08); wr8(DP(0x0F), 0x08);
}
/* As queue_bonus_tiles, for the item index in `dp` with the map bases at
 * `bases` (X = 2 * index on entry). */
static void queue_item_tiles(u8 dp, u16 bases, u16 t0, u16 t1)
{
    rep(0x20);
    lda16(rd16(ABS(RIDER_COLUMN)));
    fc = 1; sbc16(rd16(DP(0x10)));
    and16(0x0020);
    for (int i = 0; i < 5; i++) lda16(asl16(A));
    wr16(DP(0x00), A);
    lda16(rd16(ABS(RIDER_COLUMN)));
    fc = 1; sbc16(rd16(DP(0x10)));
    and16(0x001F);
    fc = 0; adc16(rd16(DP(0x00)));
    fc = 0; adc16(rd16(ABSX(bases)));
    wr16(DP(0x00), A);
    lda16(0x0018); wr16(DP(0x02), 0x0018);
    sep(0x20);
    X = rd8(DP(dp)); nz8((u8)X);
    lda8(0x21); wr8(DP(0x18), 0x21);
    lda8(rd8(ABSX(t0))); wr8(DP(0x19), (u8)A);
    lda8(rd8(ABSX(t1))); wr8(DP(0x1A), (u8)A);
    lda8(0x08); wr8(DP(0x0F), 0x08);
}
/* The port's rival pace on rider X's target speed $0236/$022E,X (the
 * registers and flags are left as they are). */
static void pace_target(void)
{
    int pct = port_rival_pace();
    if (pct == 100)
        return;
    u16 a = (u16)(X & 0xFF);
    unsigned v = (unsigned)(rd8(ABS(RIDER_TARGET_HI + a)) << 8 | rd8(ABS(RIDER_TARGET_LO + a)));
    v = v * (unsigned)pct / 100u;
    if (v > 0xFFFF) v = 0xFFFF;
    wr8(ABS(RIDER_TARGET_HI + a), (u8)(v >> 8));
    wr8(ABS(RIDER_TARGET_LO + a), (u8)v);
}
/* $80:C0E6: A is the crash speed ($08EC); the crash starts, then the tail. */
static void crash_from(void)
{
    wr8(ABSX(RIDER_HEIGHT_HI), (u8)A);
    push16(0xC0F4); StartCrash_m1x1();
    crash_tail();
}
/* $80:C0F9: the lean step $08FC from $80:BA6F by the d-pad (or $26 when the
 * jump contest's player is fast enough in the air), the height step, and
 * the landed flag $08AC once settled ($0226 = 0). */
static void crash_tail(void)
{
    lda8(rd8(ABSX(RIDER_INPUT)));
    and8(0x03);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(DP(PARALLAX_ON)));
    int fast = 0;
    if (!fz) {
        lda8(rd8(ABS(RIDER_HEIGHT_HI)));
        cmp8((u8)A, rd8(ABS(RIDER_BOUNCE_HI)));
        if (fc) {
            lda8(rd8(DP(JUMP_STATE)));
            cmp8((u8)A, 0x01);
            fast = fz;
        }
    }
    if (fast) lda8(0x26);
    else lda8(rd8(ABSY(0xBA78)));
    wr8(ABSX(RIDER_FALL_SPEED), (u8)A);
    lda8(rd8(ABSX(RIDER_Y)));
    wr8(DP(0x01), (u8)A);
    push16(0xC12B); RiderHeightStep_m1x1();
    wr8(ABSX(RIDER_Y), (u8)A);
    lda8(rd8(ABSX(RIDER_PIECE_ACTION)));
    if (fz) { lda8(0x01); wr8(ABSX(RIDER_LANDED), 0x01); }
}
/* $80:C0BD: crash at speed 0. */
static void crash_stop(void)
{
    lda8(0x00);
    crash_from();
}
/* $80:B0AF: AND A into rider X's input $017E. */
static void input_and_a(void)
{
    and8(rd8(ABSX(RIDER_INPUT)));
    wr8(ABSX(RIDER_INPUT), (u8)A);
}
/* $80:B0C8: OR Y into rider X's input $017E. */
static void input_or_y(void)
{
    lda8((u8)Y);
    ora8(rd8(ABSX(RIDER_INPUT)));
    wr8(ABSX(RIDER_INPUT), (u8)A);
}
/* $80:C2BB body: the eight start-line flag poles ($0936 - scroll $9A, Y
 * from $80:BB07, tile Y = $E1-$E3, attribute $3F; retired off the left edge
 * via $025E), then the countdown beeps: $3C at $B4, $78 or $3C plays sound
 * $26 and counts ($C3); with $54 set, $3C = $B5. */
static void start_flags(void)
{
    wr8(DP(0x00), (u8)Y);
    ldy8(0x07);
    do {
        lda8(rd8(ABSY(RIDER_RETIRED)));
        if (fz) {
            wr8(DP(0x08), 0);
            lda8(rd8(ABSY(0x0936)));
            fc = 1;
            sbc8(rd8(ABS(BG_SCROLL)));
            wr8(DP(0x01), (u8)A);
            cmp8((u8)A, 0xF0);
            if (fc) {
                cmp8((u8)A, 0xF9);
                if (fc) wr8(DP(0x08), inc8(rd8(DP(0x08))));
                lda8(0x01);
                wr8(ABSY(RIDER_RETIRED), 0x01);
            } else {
                lda8((u8)Y);
                lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
                X = (u8)A; nz8((u8)X);
                lda8(rd8(ABSY(0xBB10)));
                wr8(ABSX(OAM_BUF + 0x1A1), (u8)A);
                fc = 0; adc8(0x08);
                wr8(ABSX(OAM_BUF + 0x1A5), (u8)A);
                lda8(rd8(DP(0x00)));
                wr8(ABSX(OAM_BUF + 0x1A2), (u8)A);
                ora8(0x10);
                wr8(ABSX(OAM_BUF + 0x1A6), (u8)A);
                lda8(0x3F);
                wr8(ABSX(OAM_BUF + 0x1A3), 0x3F); wr8(ABSX(OAM_BUF + 0x1A7), 0x3F);
                lda8(rd8(DP(0x01)));
                wr8(ABSX(OAM_BUF + 0x1A0), (u8)A); wr8(ABSX(OAM_BUF + 0x1A4), (u8)A);
                lda8((u8)X);
                lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
                X = (u8)A; nz8((u8)X);
                lda8(rd8(DP(0x08)));
                wr8(ABSX(OAM_SIZE_BUF + 0x68), (u8)A);
            }
        }
        Y = dec8((u8)Y);
    } while (!fn);
    lda8(rd8(DP(0x54)));
    if (!fz) { lda8(0xB5); wr8(DP(0x3C), 0xB5); return; }
    lda8(rd8(DP(0x3C)));
    cmp8((u8)A, 0xB4);
    if (!fz) { cmp8((u8)A, 0x78); if (!fz) cmp8((u8)A, 0x3C); }
    if (fz) {
        lda8(0x26);
        push8(0x80); push16(0xC337); QueueSoundB_m1x1();
        wr8(DP(COUNTDOWN), inc8(rd8(DP(COUNTDOWN))));
    }
}
/* The rider loop of CrashRecovery (riders 7-0). Entered at $80:C279 (the
 * lean turn, enter_turn) for the current X by the player's crash steps. */
static void crash_loop(int enter_turn)
{
    if (enter_turn) goto turn;
    ldx8(0x07);
    for (;;) {
        lda8(rd8(ABSX(RIDER_BUMPED)));
        if (!fz) {
            ldy8(0x04);
            lda8((u8)X);
            ora8(rd8(ABS(ATTRACT_DEMO)));
            if (fz) {
                wr8(ABS(GAUGE_ON), (u8)A);
                lda8(rd8(DP(RIDER_TURN)));
                if (fz) {
                    lda8(0x25);
                    push8(0x80); push16(0xC22B); QueueSoundB_m1x1();
                }
                lda8(rd8(DP(0x2F)));
                and8(0x03);
                Y = (u8)A; nz8((u8)Y);
            }
            lda8((u8)Y);
            wr8(ABSX(0x0948), (u8)A);
            lda8(rd8(ABSX(RIDER_SPEED_HI)));
            if (!fz) goto turn;
            lda8(rd8(ABSX(RIDER_AIR)));
            ora8(rd8(ABSX(RIDER_SCRIPT)));
            if (!fz) goto slide;
            cmp8((u8)X, 0x00);
            if (fz) {
                lda8(rd8(ABSX(RIDER_TILE)));
                cmp8((u8)A, 0x14);
                if (fz) goto slide;
                lda8((u8)X);
            }
            wr8(ABSX(RIDER_SPEED_LO), (u8)A);
            wr8(ABSX(RIDER_STEP), (u8)A);
            wr8(ABSX(RIDER_BUMPED), (u8)A);
            lda8(0x06); wr8(ABSX(RIDER_LEAN), 0x06);
            lda8(rd8(ABSX(RIDER_X)));
            fc = 0; adc8(0x08);
            wr8(ABSX(RIDER_RUNOUT), (u8)A);
            push16(0xC267); RiderShadowY_m1x1();
            lda8(0x02); wr8(ABSX(FINISH_POSE), 0x02);
            lda8(lsr8((u8)A));
            wr8(ABSX(RIDER_TIMER), (u8)A);
            cmp8((u8)X, 0x00);
            if (fz) {
                lda8(0x3E);
                push8(0x80); push16(0xC27A); QueueSoundB_m1x1();
            }
            goto next;
        slide:
            lda8(0x88); wr8(ABSX(RIDER_SPEED_LO), 0x88);
        turn:
            lda8(rd8(DPX(RIDER_HOLD)));
            if (fz) {
                lda8(rd8(ABSX(RIDER_LEAN)));
                fc = 0; adc8(rd8(ABSX(RIDER_BUMPED)));
                and8(0x0F);
                wr8(ABSX(RIDER_LEAN), (u8)A);
            }
        }
    next:
        X = dec8((u8)X);
        if (fn) break;
    }
    ret();
}
/* The 16x16 -> 32 multiply both routines below do: [p] * [q] into r..r+3
 * (direct page), from four 8x8 products. */
static void mul16(u8 p, u8 q, u8 r, const u16 back[4])
{
    mul8(rd8(DP(p)), rd8(DP(q)), back[0]);
    lda8(rd8(ABS(RDMPYL))); wr8(DP(r), (u8)A);
    lda8(rd8(ABS(RDMPYH))); wr8(DP(r + 1), (u8)A);
    mul8(rd8(DP(p)), rd8(DP(q + 1)), back[1]);
    lda8(rd8(ABS(RDMPYL)));
    fc = 0; adc8(rd8(DP(r + 1)));
    wr8(DP(r + 1), (u8)A);
    lda8(rd8(ABS(RDMPYH)));
    adc8(0x00);
    wr8(DP(r + 2), (u8)A);
    mul8(rd8(DP(p + 1)), rd8(DP(q)), back[2]);
    lda8(rd8(ABS(RDMPYL)));
    fc = 0; adc8(rd8(DP(r + 1)));
    wr8(DP(r + 1), (u8)A);
    lda8(rd8(ABS(RDMPYH)));
    adc8(rd8(DP(r + 2)));
    wr8(DP(r + 2), (u8)A);
    mul8(rd8(DP(p + 1)), rd8(DP(q + 1)), back[3]);
    lda8(rd8(ABS(RDMPYL)));
    fc = 0; adc8(rd8(DP(r + 2)));
    wr8(DP(r + 2), (u8)A);
    lda8(rd8(ABS(RDMPYH)));
    adc8(0x00);
    wr8(DP(r + 3), (u8)A);
}
/* One 8x8 hardware multiply: WRMPYA = a, WRMPYB = b, wait (Nop_BD39 at
 * return address `back`), then the product is at RDMPYL/H. */
static void mul8(u8 a, u8 b, u16 back)
{
    lda8(a); wr8(ABS(WRMPYA), (u8)A);
    lda8(b); wr8(ABS(WRMPYB), (u8)A);
    push16(back); Nop_BD39_m1x1();
}
/* $80:B9FA Mul16b (RTS): $60:$61 * $62:$63 -> $64-$67 (direct page; the
 * IRQ calls it with D = $1B00). */
static void mul16b(void)
{
    static const u16 back[4] = { 0xBA0F, 0xBA26, 0xBA42, 0xBA5E };
    mul16(0x60, 0x62, 0x64, back);
    ret();
}
/* Queue `rows`x`cols` tiles from $80:src at VRAM `vram` (QueueTileRows). */
static void queue_rows80(u16 vram, u16 src, u16 size, u16 back)
{
    rep(0x20);
    lda16(vram); wr16(DP(0x00), vram);
    lda16(src); wr16(DP(0x02), src);
    lda16(size); wr16(DP(0x10), size);
    sep(0x20);
    lda8(0x80); wr8(DP(0x04), 0x80);
    push16(back); QueueTileRows_m1x1();
}
/* $81:CA20 body: $0C = $1569 * (Y - 1) by repeated addition (Y from
 * Hundreds), then the caller's flags back (PLP). */
static void scale_jump_tail(void)
{
    Y = dec8((u8)Y);
    rep(0x20);
    lda16(rd16(ABS(JUMP_COUNT)));
    for (;;) {
        wr16(DP(0x0C), A);
        Y = dec8((u8)Y);
        if (fn) break;
        fc = 0;
        adc16(rd16(ABS(JUMP_COUNT)));
        WAIT_TICK();
    }
    setP(pull8());
    retl();
}
static void unpack_stream(void)
{
    g_unpack_cycles = 40;
    rep(0x10);
    Y = 0; nz16(Y);
    for (;;) {
        lda8(rd8(ILDP(0x08)));
        cmp8((u8)A, 0xFF);
        if (fz) { sep(0x10); ret(); return; }
        X = rd16(DP(0x08)); X = inc16(X);
        if (fz) { X = 0x8000; nz16(X); wr8(DP(0x0A), inc8(rd8(DP(0x0A)))); }
        wr16(DP(0x08), X);
        wr8(DP(0x05), (u8)A);
        g_unpack_cycles += 75;
        cmp8((u8)A, 0xE0);
        if (!fc) {
            and8(0xE0);
            push8((u8)A);
            lda8(rd8(DP(0x05)));
            rep(0x21); fc = 0;
            and16(0x001F);
        } else {
            lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
            and8(0xE0);
            push8((u8)A);
            lda8(rd8(DP(0x05)));
            and8(0x03);
            A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
            push16(0x8C58); ReadStreamByte_m1x0();
            rep(0x21); fc = 0;
        }
        lda16(inc16(A));
        wr16(DP(0x03), A);
        lda16(Y);
        adc16(rd16(DP(0x03)));
        wr16(DP(0x05), A);
        sep(0x20);
        u8 cmd = pull8();
        lda8(cmd);
        if (fz) {                                      /* copy bytes */
            X = rd16(DP(0x08)); nz16(X);
            do {
                lda8(rd8(ILDP(0x08)));
                X = inc16(X);
                if (fz) { X = 0x8000; nz16(X); wr8(DP(0x0A), inc8(rd8(DP(0x0A)))); }
                wr16(DP(0x08), X);
                wr8((ILDP(0x00) + Y) & 0xFFFFFF, (u8)A);
                Y = inc16(Y);
                cmp16(Y, rd16(DP(0x05)));
                g_unpack_cycles += 34;
                WAIT_TICK();
            } while (!fz);
        } else if (fn) {                               /* copy from the output */
            push16(0x8CB3); ReadStreamWord_m1x0();
            X = A; nz16(X);
            do {
                push16(Y);
                Y = X; nz16(Y);
                lda8(rd8((ILDP(0x00) + Y) & 0xFFFFFF));
                Y = pull16(); nz16(Y);
                wr8((ILDP(0x00) + Y) & 0xFFFFFF, (u8)A);
                X = inc16(X);
                Y = inc16(Y);
                cmp16(Y, rd16(DP(0x05)));
                g_unpack_cycles += 37;
                WAIT_TICK();
            } while (!fz);
        } else {
            lda8(asl8((u8)A));
            if (!fn) {                                 /* $20: fill with a byte */
                push16(0x8C97); ReadStreamByte_m1x0();
                X = rd16(DP(0x03)); nz16(X);
                do {
                    wr8((ILDP(0x00) + Y) & 0xFFFFFF, (u8)A);
                    Y = inc16(Y);
                    X = dec16(X);
                    g_unpack_cycles += 14;
                    WAIT_TICK();
                } while (!fz);
            } else {
                lda8(asl8((u8)A));
                if (!fn) {                             /* $40: fill with a word */
                    push16(0x8CA4); ReadStreamWord_m1x0();
                    X = rd16(DP(0x03)); nz16(X);
                    do {
                        A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
                        wr8((ILDP(0x00) + Y) & 0xFFFFFF, (u8)A);
                        Y = inc16(Y);
                        X = dec16(X);
                        g_unpack_cycles += 17;
                        WAIT_TICK();
                    } while (!fz);
                } else {                               /* $60: incrementing fill */
                    push16(0x8C72); ReadStreamByte_m1x0();
                    X = rd16(DP(0x03)); nz16(X);
                    do {
                        wr8((ILDP(0x00) + Y) & 0xFFFFFF, (u8)A);
                        lda8(inc8((u8)A));
                        Y = inc16(Y);
                        X = dec16(X);
                        g_unpack_cycles += 16;
                        WAIT_TICK();
                    } while (!fz);
                }
            }
        }
    }
}
/* The jump result screens draw the player (DrawAllRiders, slot 0) in the
 * waving pose $19 and count $8C down; at 0 sound $42, next step ($CD) and
 * $8C = $20. Returns 1 when the step changed. */
static int jump_result_wave(u16 back)
{
    ldx8(0x00);
    push16(back); DrawAllRiders_m1x1();
    lda8(0x19); wr8(ABS(SPRITE_FRAME), 0x19);
    u8 t = dec8(rd8(DP(TIMER_B)));
    wr8(DP(TIMER_B), t);
    if (!fz) return 0;
    lda8(0x42);
    push8(0x80); push16((u16)(back + 0x0F)); QueueSoundB_m1x1();
    wr8(DP(JUMP_BOARD_STEP), inc8(rd8(DP(JUMP_BOARD_STEP))));
    lda8(0x20); wr8(DP(TIMER_B), 0x20);
    return 1;
}
/* The 32-bit accumulation both code routines do: $10-$13 += value * mult
 * (Mul16 into $04-$07, then Add32), `call` being the JSL's return address
 * (the JSR to Add32 follows it). */
static void acc_mul(u8 value, u16 mult, u16 call)
{
    rep(0x20);
    lda16(value);
    wr16(DP(0x00), A);
    lda16(mult); wr16(DP(0x02), mult);
    sep(0x20);
    push8(0x80); push16(call); Mul16_m1x1();
    push16((u16)(call + 3)); Add32_m1x1();
}
/* $80:F395: leave the demo through the table $80:F213/$80:F218 by Y
 * (JMP ($0000)). */
static void demo_exit(void)
{
    lda8(rd8(ABSY(0xF1E0))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSY(0xF1E5))); wr8(DP(0x01), (u8)A);
    r816_call_pc(0x800000u | rd16(LONG(0x000000)));
}
/* $80:ABAC: a dust puff every 16 frames; while overheated ($60 odd)
 * $00 = $58, X = 7, Y = $1C. */
static void overheat_dust(void)
{
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x0F);
    if (fz) { push8(0x80); push16(0xABBE); SpawnDustPuff_m1x1(); }
    lda8(rd8(DP(OVERHEATED)));
    lda8(lsr8((u8)A));
    if (fc) {
        lda8(0x58); wr8(DP(0x00), 0x58);
        ldx8(0x07);
        lda8((u8)X);
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
    }
}
/* $80:F8DB body: lane Y's row pointer, $07 = lane, then the piece at
 * column $08: its height byte into $10 and A = its type byte ($0180
 * further on). */
static void lane_piece(void)
{
    push16(0xF8AA); LanePieceTables_m1x1();
    wr8(DP(0x07), (u8)Y);
    Y = rd8(DP(0x08)); nz8((u8)Y);
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF));
    wr8(DP(0x10), (u8)A);
    push8((u8)Y);
    rep(0x30);
    lda16(Y);
    fc = 0; adc16(0x0180);
    Y = A; nz16(Y);
    sep(0x20);
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF));
    sep(0x10);
    Y = pull8(); nz8((u8)Y);
    ret();
}
/* $80:DC70 / $80:EF37 (two copies, base = the entry): the player against
 * the track column at its wheel ($0266). On the ground ($01EE = 0) with
 * no jump in progress, a step up taller than its speed $01B6 stops it.
 * In the air, landing at or below the column height crash-lands it
 * (stopping it first when it lands below); otherwise $0958 = 1 when the
 * piece is a $FC ramp top. */
static void player_column(u16 base)
{
    Y = rd8(ABS(RIDER_COLUMN)); nz8((u8)Y);
    lda8(rd8(ABS(RIDER_AIR)));
    if (!fz) {
        lda8(rd8(ABS(0x0904)));
        if (!fz) { ret(); return; }
        lda8(rd8(ABSY(TRACK_ROWS + 0x40)));
        Y = inc8((u8)Y);
        cmp8((u8)A, rd8(ABSY(TRACK_ROWS + 0x40)));
        if (fz) { ret(); return; }
        push16((u16)(base + 0x18)); GroundHeightAbove_m1x1();
        fc = 0; adc8(0x08);
        fc = 1; sbc8(rd8(ABSY(TRACK_ROWS + 0x40)));
        cmp8((u8)A, rd8(ABS(RIDER_Y)));
        if (fc) { ret(); return; }
        player_stop();
        ret();
        return;
    }
    lda8(rd8(ABS(0x09C7)));
    if (!fz) {
        cmp8((u8)A, rd8(ABSY(TRACK_ROWS + 0x40)));
        int stop;
        if (fz) {
            Y = inc8((u8)Y);
            cmp8((u8)A, rd8(ABSY(TRACK_ROWS + 0x40)));
            stop = !fc;
            if (!stop) goto ramp;
        } else stop = !fc;
        if (stop) { push16((u16)(base + 0x50)); player_stop(); pull16(); }
        player_crash_tail(base);
        return;
    }
ramp:
    ldx8(0x01);
    lda8(rd8(ABSY(TRACK_ROWS)));
    cmp8((u8)A, 0xFC);
    if (!fz) ldx8(0x00);
    wr8(ABS(RIDER_BAND), (u8)X);
    ret();
}
/* $80:DCC1 / $80:EF88: the player crash-lands, copying $08D4 / $08DC to
 * $08E4 / $08EC. */
static void player_crash_tail(u16 base)
{
    push16((u16)(base + 0x53)); CrashLand_m1x1();
    lda8(rd8(ABS(RIDER_BOUNCE_LO))); wr8(ABS(RIDER_HEIGHT_LO), (u8)A);
    lda8(rd8(ABS(RIDER_BOUNCE_HI))); wr8(ABS(RIDER_HEIGHT_HI), (u8)A);
    ret();
}
/* $80:DC95 / $80:EF5C: the player stops dead (flags $01CE = $0999 = $FF,
 * speed, jump height and fractions cleared). */
static void player_stop(void)
{
    lda8(0xFF);
    wr8(ABS(RIDER_BUMPED), 0xFF);
    wr8(ABS(0x0999), 0xFF);
    lda8(0x00);
    wr8(ABS(RIDER_SPEED_LO), 0); wr8(ABS(RIDER_SPEED_HI), 0); wr8(ABS(PLAYER_GROUND), 0);
    wr8(ABS(0x09C7), 0); wr8(ABS(RIDER_STEP), 0); wr8(ABS(RIDER_SLOW), 0);
}
/* $80:AFE9 / $80:B01E: CPU rider X picks a lane change. Up (AFE9) wants
 * lane $08A4 - 1 (from lane 0 it tries down instead), down (B01E) lane
 * + 1 (from lane 4, up). The lane must be flat (RiderOnFlat) and clear
 * of a rider ahead (RiderAhead at row $0958 -/+ 1); next to the far
 * lane (3 going up, 2 going down) that lane must be flat too. Then the
 * steering $01AE = +1 (up) / -1 (down). */
static void steer_lane(int up, int from_ahead)
{
    u16 b;
    if (!from_ahead) {
        for (;;) {
            Y = rd8(ABSX(RIDER_LANE_NUM)); nz8((u8)Y);
            if (up) {
                Y = dec8((u8)Y);
                cmp8((u8)Y, 0x01);
                if (fn) { up = 0; continue; }
            } else {
                Y = inc8((u8)Y);
                cmp8((u8)Y, 0x05);
                if (!fn) { up = 1; continue; }
            }
            break;
        }
        b = up ? 0xAFF2 : 0xB027;
        push16((u16)(b + 0x0A)); RiderOnFlat_m1x1();
        if (fc) { ret(); return; }
    }
    b = up ? 0xAFF2 : 0xB027;
    Y = rd8(ABSX(RIDER_BAND));
    Y = up ? inc8((u8)Y) : dec8((u8)Y);
    wr8(DP(0x02), (u8)Y);
    ldy8(0x00);
    push16((u16)(b + 0x17)); RiderAhead_m1x1();
    if (fc) { ret(); return; }
    lda8(rd8(ABSX(RIDER_LANE_NUM)));
    cmp8((u8)A, up ? 0x03 : 0x02);
    if (fz) {
        ldy8(up ? 0x02 : 0x03);
        push16((u16)(b + 0x25)); RiderOnFlat_m1x1();
        adc8(0x00);
        if (!fz) { ret(); return; }
    }
    lda8(up ? 0x01 : 0xFF);
    wr8(ABSX(RIDER_STEER), (u8)A);
    ret();
}
/* $80:DB7B / $80:EE40 (two copies, tables = the per-course pointer
 * tables): the next column ($09C3) of course $027D's lane data into
 * column $0F of lanes 0-3; lane 2's piece (0 = $3B) also fills the six
 * scenery rows $0B53-$0C93. */
static void course_column(u16 lo_table, u16 hi_table, u16 back)
{
    X = rd8(ABS(ROW_TYPE)); nz8((u8)X);
    lda8(rd8(ABSX(lo_table))); wr8(DP(0x09), (u8)A);
    lda8(rd8(ABSX(hi_table))); wr8(DP(0x0A), (u8)A);
    ldy8(0x00); wr8(DP(0x0E), 0);
    do {
        Y = rd8(DP(0x0E)); nz8((u8)Y);
        push16(back); LanePieceTables_m1x1();
        Y = rd8(ABS(BUILD_COLUMN)); nz8((u8)Y);
        lda8(rd8((((u32)DB << 16) + rd16(DP(0x09)) + Y) & 0xFFFFFF));
        wr8(ABS(BUILD_COLUMN), inc8(rd8(ABS(BUILD_COLUMN))));
        Y = rd8(DP(0x0F)); nz8((u8)Y);
        wr8((((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF, (u8)A);
        wr8(DP(0x10), (u8)A);
        wr8(DP(0x0E), inc8(rd8(DP(0x0E))));
        lda8(rd8(DP(0x0E)));
        cmp8((u8)A, 0x03);
        if (fz) {
            lda8(rd8(DP(0x10)));
            if (fz) lda8(0x3B);
            wr8(ABSY(TRACK_ROWS + 0x180), (u8)A); wr8(ABSY(TRACK_ROWS + 0x1C0), (u8)A);
            wr8(ABSY(TRACK_ROWS + 0x200), (u8)A); wr8(ABSY(TRACK_ROWS + 0x240), (u8)A);
            wr8(ABSY(TRACK_ROWS + 0x280), (u8)A); wr8(ABSY(TRACK_ROWS + 0x2C0), (u8)A);
            lda8(rd8(DP(0x0E)));
        }
        cmp8((u8)A, 0x04);
    } while (!fz);
    ret();
}
/* $80:B017: down, unless rider X's $0216 is $E6 (then up). */
static void steer_by_e6(void)
{
    lda8(rd8(ABSX(RIDER_TILE_Y)));
    cmp8((u8)A, 0xE6);
    steer_lane(fz, 0);
}
/* $80:DC95 / $80:DC9D and $80:EF5C / $80:EF64: the player stops (the
 * full stop, or only the speed and jump state cleared). */
static void player_halt(void)
{
    lda8(0x00);
    wr8(ABS(RIDER_SPEED_LO), 0); wr8(ABS(RIDER_SPEED_HI), 0); wr8(ABS(PLAYER_GROUND), 0);
    wr8(ABS(0x09C7), 0); wr8(ABS(RIDER_STEP), 0); wr8(ABS(RIDER_SLOW), 0);
    ret();
}
static u32 piece_addr(void)
{
    return (((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF;
}
/* The player's lane piece ($0180 on: its type byte), Y left 16-bit. */
static void player_piece_type(u16 back)
{
    push16(back); RiderLanePieceAt_m1x1();
    rep(0x30);
    lda16(Y);
    fc = 0; adc16(0x0180);
    Y = A; nz16(Y);
    sep(0x20);
}
/* The course map entry ($7F:4000) for the column $54 back from the
 * scroll $AE (wrapping by $B0), by the position table $7F:6000: X = the
 * entry, Y = the player's lane, 8-bit A. */
static void course_map_slot(void)
{
    rep(0x30);
    lda16(rd16(DP(COURSE_COLUMN)));
    fc = 1; sbc16(0x0054);
    if (fn) { fc = 0; adc16(rd16(DP(0xB0))); }
    X = A; nz16(X);
    lda16(rd16(LONGX(TRACK_COL_INDEX)));
    cmp16(A, rd16(LONGX(TRACK_BUF + 0x1FFE)));
    if (!fz) lda16(dec16(A));
    X = A; nz16(X);
    lda16(rd16(ABS(RIDER_LANE_NUM)));
    and16(0x00FF);
    Y = A; nz16(Y);
    sep(0x20);
}
/* $80:F96E (RTS, X = rider): jump height $01FE = $026E - A. */
static void height_below(void)
{
    wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSX(RIDER_JUMP_BASE)));
    fc = 1; sbc8(rd8(DP(0x00)));
    wr8(ABSX(RIDER_HEIGHT), (u8)A);
    ret();
}
/* $80:BFDB: rider X crashes on landing: sound $3D for the player, $3D,X
 * = $0A, crashed ($01CE), then sound $24 (player) / $29 if on screen
 * page 1. */
static void landing_crash(void)
{
    cmp8((u8)X, 0x00);
    if (fz) {
        lda8(0x3D);
        push8(0x80); push16(0xBFED); QueueSoundB_m1x1();
    }
    lda8(0x0A); wr8(DPX(RIDER_HOLD), 0x0A);
    lda8(0xFF); wr8(ABSX(RIDER_BUMPED), 0xFF);
    lda8((u8)X);
    if (!fz) lda8(0x05);
    fc = 0; adc8(0x24);
    wr8(DP(0x10), (u8)A);
    lda8(rd8(ABSX(RIDER_PAGE)));
    cmp8((u8)A, 0x01);
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0x10)));
    push8(0x80); push16(0xC00D); QueueSoundB_m1x1();
    ret();
}
/* $81:C018 / $81:C085: object $03A3's frame sequence. When its timer
 * $036F runs out it steps $031F (timer from the table at times), and on
 * frame `spawn` spawns a type-8 object (state 1, $032F = 4) at its
 * position less ($16, dy); past the last frame it goes to state 4 with
 * a timer of 8. Its frame $034F comes from the table at frames. */
static void frame_sequence(u8 count, u16 times, u8 spawn, u16 dy,
                           u16 frames, u16 back)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(ABSX(OBJ_DELAY)));
    if (fz) {
        lda8(rd8(ABSX(OBJ_STEP)));
        lda8(inc8((u8)A));
        cmp8((u8)A, count);
        if (fc) {
            lda8(0x04); wr8(ABSX(OBJ_STATE), 0x04);
            lda8(0x08); wr8(ABSX(OBJ_DELAY), 0x08);
            wr8(ABSX(OBJ_STEP), 0);
            ret();
            return;
        }
        wr8(ABSX(OBJ_STEP), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSY(times)));
        wr8(ABSX(OBJ_DELAY), (u8)A);
        lda8(rd8(ABSX(OBJ_STEP)));
        cmp8((u8)A, spawn);
        if (fz) {
            push8((u8)Y);
            rep(0x20);
            lda16(rd16(ABSX(OBJ_X)));
            fc = 1; sbc16(0x0016);
            wr16(DP(0x00), A);
            lda16(rd16(ABSX(OBJ_Y)));
            fc = 1; sbc16(dy);
            wr16(DP(0x02), A);
            sep(0x20);
            ldy8(0x08);
            push8(0x81); push16(back); SpawnObject_m1x1();
            Y = rd8(DP(0x0E)); nz8((u8)Y);
            lda8(0x01); wr8(ABSY(OBJ_STEP), 0x01);
            lda8(0x04); wr8(ABSY(OBJ_RANDOM), 0x04);
            Y = pull8(); nz8((u8)Y);
        }
    }
    Y = rd8(ABSX(OBJ_STEP)); nz8((u8)Y);
    lda8(rd8(ABSY(frames)));
    wr8(ABSX(OBJ_FRAME), (u8)A);
    ret();
}
/* $80:DBFC / $80:EEBB (two copies; b = the second, which also silences
 * APU port 2 and clears $01C6 on the get-up): the crashed player
 * ($01CE). Still tumbling ($0999): on the ground, stopped and level
 * (angle 6, $0176 clear) it gets up; otherwise it slides at $88 and,
 * once $3B is clear, sets $3B = 2 and turns its lean (CrashRecovery's
 * loop from $80:C279). Stopped: the speed is cleared and the timer $3A
 * starts at $20; below $1C it slides, and at 1 the tumble ends: $0999 =
 * 1, $09C4 = 0 and (unless $0176) the player halts and crash-lands. */
static void player_crash_step(int b)
{
    u16 base = b ? 0xEF37 : 0xDC79;
    lda8(rd8(ABS(RIDER_BUMPED)));
    if (fz) { ret(); return; }
    lda8(0x00); wr8(ABS(GAUGE_ON), 0x00);
    lda8(rd8(DP(RIDER_TURN)));
    if (fz) lda8(0x01);
    lda8(rd8(ABS(0x0999)));
    if (!fz) {
        lda8(rd8(ABS(RIDER_AIR)));
        if (fz) {
            lda8(rd8(ABS(RIDER_SPEED_HI)));
            if (fz) {
                lda8(rd8(ABS(RIDER_SCRIPT)));
                int up = 0;
                if (fz) {
                    lda8(rd8(ABS(RIDER_LEAN)));
                    cmp8((u8)A, 0x06);
                    up = fz;
                }
                if (up) {
                    if (b) { lda8(0x00); wr8(ABS(APUIO2), 0x00); }
                    lda8(0x00);
                    wr8(ABS(RIDER_SPEED_LO), 0);
                    if (b) wr8(ABS(RIDER_SPEED_HI), 0);
                    wr8(ABS(RIDER_BUMPED), 0);
                    wr8(ABS(0x0999), 0);
                    ret();
                    return;
                }
                lda8(0x88); wr8(ABS(RIDER_SPEED_LO), 0x88);
            }
        }
        goto slide;
    }
    lda8(0x00);
    wr8(ABS(RIDER_SPEED_LO), 0); wr8(ABS(RIDER_SPEED_HI), 0); wr8(ABS(RIDER_STEP), 0);
    lda8(rd8(DP(0x3A)));
    if (fz) { lda8(0x20); wr8(DP(0x3A), 0x20); }
    cmp8((u8)A, 0x1C);
    if (fc) { ret(); return; }
    cmp8((u8)A, 0x01);
    if (!fz) goto slide;
    wr8(ABS(0x0999), (u8)A);
    lda8(lsr8((u8)A));
    wr8(ABS(PLAYER_GROUND), (u8)A);
    lda8(rd8(ABS(RIDER_SCRIPT)));
    if (!fz) { ret(); return; }
    push16(b ? 0xEF33 : 0xDC75); player_halt();
    player_crash_tail(base);
    return;
slide:
    lda8(rd8(DP(0x3B)));
    if (!fz) { ret(); return; }
    lda8(0x02); wr8(DP(0x3B), 0x02);
    crash_loop(1);
}
/* $80:96B1: add the result points to the score, sound $43, and the score
 * ($0CE5, six digits) at VRAM $1B16. */
static void results_score_tail(void)
{
    push16(0x96B1); AddScore_m1x1();
    lda8(0x43);
    push8(0x80); push16(0x96B7); QueueSoundB_m1x1();
    rep(0x20);
    lda16(0x0CE5); wr16(DP(0x02), 0x0CE5);
    lda16(0x1B16); wr16(DP(0x00), 0x1B16);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    ldy8(0x05);
    push8(0x80); push16(0x96CF); QueueDigitsAt_m1x1();
    ret();
}
/* $80:C083: lean rider X one step toward angle A, every Y frames ($45,X). */
static void lean_toward(void)
{
    wr8(DP(0x00), (u8)A);
    lda8(rd8(DPX(RIDER_LEAN_TIMER)));
    if (!fz) { ret(); return; }
    lda8((u8)Y);
    wr8(DPX(RIDER_LEAN_TIMER), (u8)A);
    lda8(rd8(ABSX(RIDER_LEAN)));
    cmp8((u8)A, rd8(DP(0x00)));
    if (fz) { ret(); return; }
    if (fc) {
        wr8(ABSX(RIDER_LEAN), dec8(rd8(ABSX(RIDER_LEAN))));
        wr8(ABSX(RIDER_LEAN), dec8(rd8(ABSX(RIDER_LEAN))));
    }
    wr8(ABSX(RIDER_LEAN), inc8(rd8(ABSX(RIDER_LEAN))));
    ret();
}
/* $80:AEDB: the d-pad leans rider X (every $80:B889 frames by air state):
 * up ($00 bit 0) leans back to $80:B87D; otherwise forward to $80:B87F,
 * and holding a button past it (on the ground) keeps tipping until angle
 * $0D flips it ($01CE = 1, $3D,X = $0A, sound $24 / $29). */
static void lean_by_pad(void)
{
    lda8(rd8(DPX(RIDER_LEAN_TIMER)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_AIR)));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xB892)));
    wr8(DPX(RIDER_LEAN_TIMER), (u8)A);
    {
        u8 v = rd8(DP(0x00));
        fc = v & 1;
        v >>= 1;
        wr8(DP(0x00), v);
        nz8(v);
    }
    if (fc) {
        lda8(rd8(ABSX(RIDER_LEAN)));
        cmp8((u8)A, rd8(ABSY(0xB886)));
        if (fz) { ret(); return; }
        if (!fc) { wr8(ABSX(RIDER_LEAN), inc8(rd8(ABSX(RIDER_LEAN)))); ret(); return; }
        wr8(ABSX(RIDER_LEAN), dec8(rd8(ABSX(RIDER_LEAN))));
        ret();
        return;
    }
    lda8(rd8(ABSX(RIDER_SLOW)));
    and8(0x02);
    wr8(ABSX(RIDER_SLOW), (u8)A);
    lda8(rd8(ABSX(RIDER_LEAN)));
    cmp8((u8)A, rd8(ABSY(0xB888)));
    if (!fc) { wr8(ABSX(RIDER_LEAN), inc8(rd8(ABSX(RIDER_LEAN)))); ret(); return; }
    lda8(rd8(ABSX(RIDER_INPUT)));
    and8(0xC0);
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_AIR)));
    if (!fz) { ret(); return; }
    wr8(ABSX(RIDER_LEAN), inc8(rd8(ABSX(RIDER_LEAN))));
    lda8(0x0D); wr8(DPX(RIDER_LEAN_TIMER), 0x0D);
    lda8(rd8(ABSX(RIDER_LEAN)));
    cmp8((u8)A, 0x0D);
    if (!fc) { ret(); return; }
    lda8(0x01); wr8(ABSX(RIDER_BUMPED), 0x01);
    lda8(0x0A); wr8(DPX(RIDER_HOLD), 0x0A);
    lda8((u8)X);
    if (!fz) lda8(0x05);
    fc = 0; adc8(0x24);
    push8(0x80); push16(0xAF46); QueueSoundB_m1x1();
    ret();
}
/* $80:9713: the course records list (before the results, $7D = 0): every
 * $20 frames ($0088) the next of up to ten times ($0819, three bytes;
 * $FF ends the list) at its VRAM row ($80:97A5) with sound $42, sound
 * $43 after the tenth; past ten the course record itself. Leaving ($8A
 * runs out, or Start with the debug flag $81:800E) goes to the results
 * (mode 2, $7D + 1). */
static void course_records_step(void)
{
    X = rd8(DP(COURSE_SLOT)); nz8((u8)X);
    rep(0x20);
    wr16(DP(TIMER_A), dec16(rd16(DP(TIMER_A))));
    if (!fz) {
        sep(0x20);
        lda8(rd8(LONG(DEBUG_ENABLED)));
        int leave = 0;
        if (!fz) {
            lda8(rd8(DP(P1_PRESS_HI)));
            and8(0x10);
            leave = !fz;
        }
        if (!leave) {
            wr8(ABS(MODE_TIMER), dec8(rd8(ABS(MODE_TIMER))));
            if (!fz) { ret(); return; }
            lda8(0x20); wr8(ABS(MODE_TIMER), 0x20);
            lda8(rd8(ABS(ENTRY_CURSOR)));
            cmp8((u8)A, 0x0A);
            if (fc) { push16(0x973A); ShowCourseRecord_m1x1(); ret(); return; }
            lda8(asl8((u8)A));
            X = (u8)A; nz8((u8)X);
            fc = 0; adc8(rd8(ABS(ENTRY_CURSOR)));
            Y = (u8)A; nz8((u8)Y);
            lda8(rd8(ABSY(0x0819)));
            cmp8((u8)A, 0xFF);
            if (!fz) {
                wr8(ABS(RACE_MIN), (u8)A);
                lda8(rd8(ABSY(0x081A))); wr8(ABS(RACE_SEC), (u8)A);
                lda8(rd8(ABSY(0x081B))); wr8(ABS(RACE_FRAMES), (u8)A);
                lda8(0x42);
                push8(0x80); push16(0x976E); QueueSoundB_m1x1();
                rep(0x20);
                lda16(rd16(ABSX(0x97A3))); wr16(DP(0x00), A);
                sep(0x20);
                push16(0x977A); TickRaceClock_m1x1();
            } else {
                lda8(rd8(ABS(ENTRY_CURSOR)));
                if (fz) {
                    lda8(0x0A); wr8(ABS(ENTRY_CURSOR), 0x0A);
                    lda8(0x01); wr8(ABS(MODE_TIMER), 0x01);
                    ret();
                    return;
                }
                lda8(0x09); wr8(ABS(ENTRY_CURSOR), 0x09);
            }
            wr8(ABS(ENTRY_CURSOR), inc8(rd8(ABS(ENTRY_CURSOR))));
            lda8(rd8(ABS(ENTRY_CURSOR)));
            cmp8((u8)A, 0x0A);
            if (fz) { lda8(0x43); push8(0x80); push16(0x97A1); QueueSoundB_m1x1(); }
            ret();
            return;
        }
    }
    sep(0x20);
    lda8(0x02); wr8(DP(GAME_MODE), 0x02);
    wr8(DP(MODE_ACTIVE), 0); wr8(DP(MODE_STEP), 0);
    wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
    wr8(DP(FINAL_RACE), inc8(rd8(DP(FINAL_RACE))));
    ret();
}
static u8 course_list4(u32 dp)
{
    return rd8((((u32)DB << 16) + rd16(DP(dp)) + Y) & 0xFFFFFF);
}
/* ($0A),Y in bank DB. */
static u8 script_byte4(void)
{
    return rd8((((u32)DB << 16) + rd16(DP(0x0A)) + Y) & 0xFFFFFF);
}
/* $80:A9C6 / $80:F2E5: lean and lane band for riders 7-0. */
static void lean_and_bands4(u16 back1, u16 back2)
{
    ldx8(0x07);
    do {
        push16(back1); SetRiderLean_m1x1();
        push16(back2); RiderLaneBand_m1x1();
        X = dec8((u8)X);
    } while (!fn);
}
/* Fill `bytes` VRAM bytes of one half from the byte at $00:0000 through
 * channel 0 (D = $4300), as Stadium 1's ClearTilemaps. */
static void vram_fill_half4(u16 vram, int wide_stz, u16 bytes)
{
    if (wide_stz) wr16(ABS(VMAIN), 0);
    else wr8(ABS(VMAIN), 0);
    wr16(ABS(VMADDL), vram);
    wr16(DP(0x00), 0x1808);
    wr16(DP(0x02), 0);
    wr16(DP(0x04), 0);
    wr16(DP(0x05), bytes);
    wr8(ABS(MDMAEN), 0x01);
}
/* $80:9F17 / $80:9CDD: leave for the BS-X BIOS: interrupts and NMI off,
 * the BS-X memory mapping reset, the sound CPU back to its loader, JML
 * $10:5C3C. The port never runs this (port/src/gen_stubs.c replaces both
 * routines' exits). */
static void bsx_exit4(u32 site)
{
    if (port_bsx_exit(site))            /* the port's exit instead (roster.h) */
        return;
    fi = 1;
    wr8(ABS(NMITIMEN), 0);
    lda8(0x00);
    wr8(LONG(0x035000), 0);
    wr8(LONG(0x045000), 0);
    wr8(LONG(0x075000), 0);
    wr8(LONG(0x085000), 0);
    lda8(0x80); wr8(LONG(0x0E5000), 0x80);
    lda8(0xFE); wr8(ABS(APUIO0), 0xFE);
    if (!r816_call_pc(0x105C3C))
        r816_unknown_target(site, 0x105C3C);
}
/* $81:F999 (JSR, keeps A): restart the minute's frame counter; X = 1 marks
 * "clock taken from the stream this frame". Stack traffic as the original. */
static void restart_minute4(u16 ret_addr, u8 a)
{
    push16(ret_addr);
    push8(a);
    wr16(LONG(BCAST_FRAME), 0);
    X = 1;
    pull8();
    pull16();
}
/* The riders' start state shared by the demo and course-select setups:
 * $54 = 5, $0CD5 = $A0, lanes $0936-$093D = $50..$C0, the player's
 * pieces and positions. */
static void course_start_riders4(int select)
{
    lda8(0x05); wr8(DP(0x54), 0x05);
    lda8(0xA0); wr8(ABS(GROUND_LINE), 0xA0);
    ldx8(0x07);
    lda8(0xC0);
    do {
        wr8(ABSX(0x0936), (u8)A);
        fc = 1; sbc8(0x10);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x0E); wr8(ABS(RIDER_LANE), 0x0E);
    lda8(0x08); wr8(ABS(RIDER_WHEEL), 0x08);
    lda8(0x09); wr8(ABS(RIDER_COLUMN), 0x09);
    lda8(0x38); wr8(ABS(RIDER_X), 0x38);
    if (select) {
        lda8(0xB8); wr8(ABS(RIDER_Y), 0xB8);
        wr8(ABS(0x0904), 0);
    }
    lda8(0x06); wr8(ABS(RIDER_LEAN), 0x06);
    if (select) wr8(ABS(RIDER_PIECE_LEAN), 0x06);
    wr8(ABS(RIDER_TARGET_HI), 0x06);
    lda8(0x01); wr8(ABS(RIDER_PAGE), 0x01); wr8(ABS(RIDER_ACTIVE), 0x01);
    lda8(0x01); wr8(ABS(0x08C4), 0x01);
    wr8(DP(0x94), 0x01);
    wr8(DP(MODE_ACTIVE), 0x01);
    lda8(0x01); wr8(ABS(FADE_STATE), 0x01);
    lda8(0x19); wr8(ABS(BG3SC), 0x19);
    lda8(0x01); wr8(ABS(BG34NBA), 0x01);
    lda8(0x17); wr8(ABS(SH_TM), 0x17);
}
/* Tail jump through a 16-bit table (same bank) indexed by 2 * v. */
static void table_jump4(u32 table, u8 v, u32 site)
{
    lda8(v);
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    u16 t = rd16(LONG(table + X));
    if (!r816_call_pc((table & 0xFF0000) | t))
        r816_unknown_target(site, t);
}
/* Object $03A3's state $030F through a table (RTS by tail): $81:9835,
 * $81:B8E3, $81:B95D, $81:BA4C, $81:BB80, $81:BCC1, $81:BDDC, $81:BF34,
 * $81:BFFC. */
static void object_state_jump4(u32 table, u32 site)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    table_jump4(table, rd8(ABSX(OBJ_STATE)), site);
}
/* $80:F9FA: $0206,X = A. */
static void set_jump_state4(void)
{
    wr8(ABSX(RIDER_LANDING), (u8)A);
    ret();
}
/* $80:F375: the next recorded demo entry (as DemoInputStep's): a duration
 * of $F8 or more ends the demo through exit $FF - it; else the duration
 * and input are taken, C clear. */
static void demo_next_entry4(void)
{
    X = rd8(ABS(DEMO_INDEX)); nz8((u8)X);
    wr8(ABS(DEMO_INDEX), inc8(rd8(ABS(DEMO_INDEX))));
    lda8(rd8(LONGX(0x9CD300)));
    cmp8((u8)A, 0xF8);
    if (fc) {
        eor8(0xFF);
        Y = (u8)A; nz8((u8)Y);
        demo_exit();
        return;
    }
    wr8(ABS(DEMO_TIME), (u8)A);
    lda8(rd8(LONGX(0x9CD200)));
    wr8(ABS(RIDER_INPUT), (u8)A);
    wr8(ABS(DEMO_PAD), (u8)A);
    fc = 0;
    ret();
}
/* $81:C659: send the engine pitch $198A to APU port 2. */
static void send_pitch4(void)
{
    lda8(rd8(ABS(ENGINE_PITCH)));
    wr8(ABS(APUIO2), (u8)A);
    ret();
}
/* Interrupt return: P, then PC and PB from the interrupt frame. */
static void rti4(void) { setP(pull8()); pull16(); pull8(); }
/* One sound command channel in NMI (as Stadium 1's): wait for APU port n
 * to echo the pending command $6D+n, then step the ring index $69+n; with
 * none pending, send the next one from the ring $1400 + $40n. */
static void nmi_sound_channel4(u8 n)
{
    lda8(rd8(DP(SND_PENDING + n)));
    if (!fz) {
        cmp8((u8)A, rd8(ABS(APUIO0 + n)));
        if (!fz) return;
        lda8(rd8(DP(SND_READ + n)));
        lda8(inc8((u8)A));
        and8(0x3F);
        wr8(DP(SND_READ + n), (u8)A);
        lda8(0x00);
    } else {
        X = rd8(DP(SND_READ + n)); nz8((u8)X);
        lda8(rd8(ABSX(SND_RING + 0x40 * n)));
        if (fz) return;
        wr8(ABSX(SND_RING + 0x40 * n), 0);
    }
    wr8(DP(SND_PENDING + n), (u8)A);
    wr8(ABS(APUIO0 + n), (u8)A);
}
/* $80:8108-$80:81A6: one main loop frame after NMI ($0817 set): minute to
 * $BF, the BS-X stream, the random shift register, the joypads, IRQs on.
 * In the debug build ($81:800E) pad 2's L/R step the broadcast minute
 * (restarting it) and Select / Start toggle a pause ($1917) that skips the
 * game logic. Then a fade step, or the frame counter $63, timers, the minute
 * marker, hidden sprites and the game mode script; the race's lanes and
 * frame step when $94; rider sprite sets, sprite sizes, the clock;
 * $0817 cleared. */
R816_EXPORT void MainLoopFrame4(void)
{
    lda8(rd8(LONG(BCAST_MINUTE)));
    wr8(ABS(0x00BF), (u8)A);
    push8(0x80); push16(0x8112); BsxPollStream_m1x1();
    push16(0x8115); StepLfsr_m1x1();
    push8(0x80); push16(0x8119); ReadJoypads_m1x1();
    fi = 0;
    lda8(rd8(LONG(DEBUG_ENABLED)));
    if (!fz) {
        lda8(rd8(DP(P2_PRESS_LO)));
        and8(0xC0);
        if (!fz) {
            lda8(asl8((u8)A));
            lda8(rd8(LONG(BCAST_MINUTE)));
            if (fc) {
                lda8(dec8((u8)A));
                if (fn) lda8(0x00);
            } else {
                lda8(inc8((u8)A));
            }
            wr8(LONG(BCAST_MINUTE), (u8)A);
            wr8(ABS(0x00B8), inc8(rd8(ABS(0x00B8))));
            lda8(0x01); wr8(ABS(0x1E84), 0x01);
            rep(0x20);
            lda16(0x0000); wr16(LONG(BCAST_FRAME), 0);
            sep(0x20);
        }
        lda8(rd8(DP(P1_PRESS_LO)));
        and8(0x20);
        if (!fz) wr8(ABS(DEBUG_PAUSE), inc8(rd8(ABS(DEBUG_PAUSE))));
        lda8(rd8(DP(P1_PRESS_LO)));
        and8(0x10);
        if (fz) {
            lda8(rd8(ABS(DEBUG_PAUSE)));
            and8(0x01);
            wr8(ABS(DEBUG_PAUSE), (u8)A);
            if (!fz) goto paused;
        }
    }
    lda8(rd8(ABS(FADE_STATE)));
    if (!fz) {
        push8(0x80); push16(0x8176); StepFade_m1x1();
        goto tail;
    }
    rep(0x20);
    wr16(DP(FRAME_COUNT), inc16(rd16(DP(FRAME_COUNT))));
    sep(0x20);
    push8(0x80); push16(0x8182); TickTimers_m1x1();
    push8(0x80); push16(0x8186); PlaceMinuteMarker_m1x1();
    push8(0x80); push16(0x818A); HideAllSprites_m1x1();
    push16(0x818D); RunGameModeScript_m1x1();
paused:
    lda8(rd8(DP(0x94)));
    if (!fz) {
        push16(0x8194); LaneFromY_m1x1();
        push16(0x8197); RaceFrameStep_m1x1();
    }
tail:
    push8(0x80); push16(0x819B); RiderSpriteSets_m1x1();
    push8(0x80); push16(0x819F); PackSpriteSizes_m1x1();
    push16(0x81A2); DrawClockB_m1x1();
    wr8(ABS(NMI_FRAME_READY), 0);
}
/* The boot alone, and one turn of the main loop (a frame if the NMI has
 * flagged 0817): the port's frame-loop native mode calls these once and
 * once per frame instead of the endless loop (port/src/native_driver.c). */
R816_EXPORT void ResetBootAll(void)
{
    ResetBoot4a();
    ResetBoot4aSpc();
    wait_apu_idle4();
    ResetBoot4b();
}
/* $80:8089-$80:80E9: the second SPC upload (set 2), the sound state; DB = $80; IRQ
 * timer at V $D0 / H $98, FastROM, NMI on; APU port 0 = $FC; clock and
 * game state defaults; game mode $0D. */
R816_EXPORT void ResetBoot4b(void)
{
    ldx8(0x02);
    push8(0x80); push16(0x808E); SpcUploadB_m1x1();
    push8(0x80); push16(0x8092); ResetSoundState_m1x1();
    lda8(0x80); push8(0x80); DB = pull8(); nz8(DB);
    rep(0x20);
    lda16(0x00D0); wr16(ABS(VTIMEL), A);
    lda16(0x0098); wr16(ABS(HTIMEL), A);
    sep(0x20);
    lda8(0x01); wr8(ABS(MEMSEL), 0x01);
    lda8(0x81); wr8(ABS(NMITIMEN), 0x81);
    wr8(ABS(SH_NMITIMEN), 0x81);
    wr8(ABS(APUIO1), 0);
    wr8(ABS(APUIO2), 0);
    wr8(ABS(APUIO3), 0);
    lda8(0xFC); wr8(ABS(APUIO0), 0xFC);
    wr8(ABS(0x1E87), 0);
    wr8(ABS(0x1E88), 0);
    wr8(ABS(0x1566), 0);
    lda8(0x04); wr8(ABS(RECORD_TIME), 0x04);
    wr8(ABS(RECORD_TIME + 0x01), 0);
    wr8(ABS(RECORD_TIME + 0x02), 0);
    lda8(0x04);
    wr8(ABS(0x083D), 0x04);
    wr8(ABS(0x0840), 0x04);
    wr8(ABS(0x0843), 0x04);
    wr8(ABS(0x0846), 0x04);
    lda8(0x0D); wr8(ABS(GAME_MODE), 0x0D);
}
/* $80:8078: NMI off until all four APU ports read 0. */
static void wait_apu_idle4(void)
{
    do {
        wr8(ABS(NMITIMEN), 0);
        lda8(rd8(ABS(APUIO0)));
        ora8(rd8(ABS(APUIO1)));
        ora8(rd8(ABS(APUIO2)));
        ora8(rd8(ABS(APUIO3)));
    } while (!fz);
}
/* $80:8072: the first SPC upload (set 0). */
R816_EXPORT void ResetBoot4aSpc(void)
{
    ldx8(0x00);
    push8(0x80); push16(0x8077); SpcUploadB_m1x1();
}
/* $80:8000-$80:8077: native mode, D = 0, stack $1FFF; BS-X detection (the
 * time channel when $7F:FFE1 is clear); broadcast clock minute 2, frame 2,
 * $7F:FFF9 = $13, $7F:FFFB = 0; DB = 0; NMI, DMA and APU ports off, forced
 * blank, the PPU registers. */
R816_EXPORT void ResetBoot4a(void)
{
    fi = 1;
    fc = 0;                                 /* CLC; XCE (native mode) */
    rep(0x30);
    lda16(0x0000); D = A; nz16(D);
    lda16(0x1FFF); S = A;
    sep(0x30);
    push8(0x80); push16(0x8012); BsxDetect_m1x1();
    lda8(rd8(LONG(NO_RECEIVER)));
    if (fz) { push8(0x80); push16(0x801C); BsxOpenTimeChannel_m1x1(); }
    lda8(0x13); wr8(LONG(BCAST_HOUR), 0x13);
    lda8(0x02); wr8(LONG(BCAST_MINUTE), 0x02); wr8(LONG(BCAST_FRAME), 0x02);
    lda8(0x00); wr8(LONG(BCAST_FRAME_HI), 0x00);
    lda8(0x1E); wr8(ABS(0x1E84), 0x1E);
    wr8(ABS(0x1E85), 0);
    sep(0x20);
    lda8(0x00); push8(0x00); DB = pull8(); nz8(DB);
    wr8(ABS(NMITIMEN), 0);
    wr8(ABS(HDMAEN), 0);
    wr8(ABS(MDMAEN), 0);
    wr8(ABS(APUIO0), 0);
    wr8(ABS(APUIO1), 0);
    wr8(ABS(APUIO2), 0);
    wr8(ABS(APUIO3), 0);
    lda8(0x80); wr8(ABS(INIDISP), 0x80);
    push8(0x80); push16(0x805E); InitPpuRegisters_m1x1();
    lda8(0x00);
    wr8(ABS(0x1E1C), 0);
    wr8(DP(0xF2), 0);
    wr8(DP(0xF4), 0);
    lda8(0x00);
    wr8(LONG(0x7FFFE3), 0);
    wr8(LONG(0x7FFFE4), 0);
}

/*
 * $82:8965 RiderGfxCycle (RTS): while $1E0C is set, every other frame queue
 * the next of 16 animation strips: source $C000 + $82:8940[n] ($1E0A) to
 * VRAM $4F00 + (n & 3) * $100 ($1E10), n = $1E0E counting up.
 */
void RiderGfxCycle_m1x1(void)
{
    rep(0x20);
    lda16(rd16(ABS(ANIM_UPLOAD_ON)));
    if (!fz) {
        wr16(ABS(0x1E10), 0);
        wr16(ABS(0x1E0E), inc16(rd16(ABS(0x1E0E))));
        lda16(rd16(ABS(0x1E0E)));
        and16(0x0001);
        if (fz) {
            lda16(rd16(ABS(0x1E0E)));
            and16(0x001E);
            X = (u8)A; nz8((u8)X);
            lda16(rd16(ABS(0x8940 + X)));
            fc = 0;
            adc16(0xC000);
            wr16(ABS(0x1E0A), A);
            lda16((u16)X);
            and16(0x0006);
            lda16(lsr16(A));
            A = (u16)((A << 8) | (A >> 8)); nz8((u8)A);   /* XBA */
            fc = 0;
            adc16(0x4F00);
            wr16(ABS(0x1E10), A);
        }
    }
    sep(0x20);
    ret();
}
/*
 * $82:87FD RiderSpriteSets (RTL), every frame: set the four rider sprite
 * slots' animation sources. Slot s shows frame f = $1961,s of its rider set
 * (roster_rider_offset/bank for the player's character $154F and the race
 * variant $1E91): $1E12+2s = set offset + (f & $FE) * $200 + (f & 1) * $100,
 * bank $1550+s. Slot 0 frame $15 is replaced by $82:8960[$091C]. With a fade
 * running ($1923) or in a mode without riders ($82:928E[$76] = 0) the
 * streams are switched off.
 */
void RiderSpriteSets_m1x1(void)
{
    push8(DB);
    lda8(0x82);
    push8(0x82);
    DB = pull8(); nz8(DB);
    push16(0x8804); RiderGfxCycle_m1x1();
    lda8(rd8(ABS(FADE_STATE)));
    int on = 0;
    if (fz) {
        X = rd8(DP(GAME_MODE));
        lda8(rd8(ABS(0x928E + X)));
        on = !fz;
    }
    if (!on) {
        rep(0x20);
        for (int a = 0x1E12; a <= 0x1E18; a += 2)
            wr16(ABS(a), 0);
        sep(0x20);
        DB = pull8(); nz8(DB);
        retl();
        return;
    }
    rep(0x20);
    ldx8(0x06);
    ldy8(0x03);
    u8 ch = rd8(ABS(PLAYER_CHAR)), var = rd8(ABS(RACE_VARIANT));
    do {
        lda16(rd16(ABS(SPRITE_FRAME + Y)));
        wr16(DP(0x0E), A);
        if (Y == 0) {
            and16(0x00FF);
            cmp16(A, 0x0015);
            if (fz) {
                lda16(rd16(ABS(FINISH_POSE + Y)));
                push8((u8)X);
                X = (u8)A;
                lda16(rd16(LONG(0x828960 + X)));
                X = pull8();
                wr16(DP(0x0E), A);
            }
        }
        and16(0x0001);
        A = (u16)((A << 8) | (A >> 8)); nz8((u8)A);       /* XBA */
        wr16(DP(0x00), A);
        wr16(ABS(0x000C), 0);
        wr8(DP(0x0C), (u8)X);
        /* X = ch * 8 + slot * 2: the original's table index */
        lda16((u16)(ch << 3));
        fc = 0;
        adc16(rd16(DP(0x0C)));
        X = (u8)A; nz8((u8)X);
        u8 slotx = (u8)(X - (ch << 3));               /* byte index in the record: slot * 2 */
        lda16(rd16(ABS(RACE_VARIANT)) & 0x00FF);
        lda16(roster_rider_offset(ch, var, slotx));
        wr16(DP(0x1E), A);
        lda16(rd16(DP(0x0E)));
        and16(0x00FE);
        lda16(asl16(A));
        A = (u16)((A << 8) | (A >> 8)); nz8((u8)A);       /* XBA */
        fc = 0;
        adc16(rd16(DP(0x00)));
        fc = 0;
        adc16(rd16(DP(0x1E)));
        X = rd8(DP(0x0C));
        wr16(ABS(0x1E12 + X), A);
        X = dec8((u8)X);
        X = dec8((u8)X);
        Y = dec8((u8)Y);
    } while (!fn);
    sep(0x20);
    lda8(rd8(ABS(PLAYER_CHAR)));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABS(RACE_VARIANT)));
    for (int s = 0; s < 4; s++) {
        lda8(roster_rider_bank(ch, var, s));
        wr8(ABS(0x1550 + s), (u8)A);
    }
    DB = pull8(); nz8(DB);
    retl();
}
/*
 * $82:899B LoadRiderPalettes (RTL): copy the five rider palettes of the
 * player's character (roster_rider_palette: Stadium, race variant,
 * character) to the palette buffer $1D20-$1DBF (sprite palettes 1-5) and
 * request the upload ($1915).
 */
void LoadRiderPalettes_m1x1(void)
{
    push8(DB);
    lda8(0x82);
    push8(0x82);
    DB = pull8(); nz8(DB);
    int st = rd8(LONG(STADIUM_SELECT)) != 0;
    int var = rd8(ABS(RACE_VARIANT)) != 0;
    u8 ch = rd8(ABS(PLAYER_CHAR));
    /* $00 = st * 100 + var * 50 + ch * 10 (8-bit), as the original leaves it */
    wr8(DP(0x00), (u8)((st ? 100 : 0) + (var ? 50 : 0) + ch * 10));
    wr8(DP(0x01), 0);
    rep(0x30);
    Y = 0;
    for (int i = 0; i < 5; i++) {
        lda16((u16)(i * 2));
        fc = 0;
        adc16(rd16(DP(0x00)));                      /* index into $82:8A08 (sets V) */
        u32 pal = roster_rider_palette(st, var, ch, i);
        wr16(DP(0x02), (u16)pal);
        push16((u16)i);
        X = (u16)(i * 32);
        for (int k = 0; k < 32; k += 2) {
            lda16(rd16(LONG(pal + k)));
            wr16(ABS(PALETTE_BUF + 0x120 + X), A);
            X += 2;
        }
        wr16(DP(0x04), 0);
        Y = pull16();
        Y++;
        cmp16(Y, 0x0005);
    }
    sep(0x30);
    wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:B2E3 VramQueueTiles20: VramQueueTiles with attribute high byte $20. */
void VramQueueTiles20_m1x1(void)
{
    wr8(DP(0x0F), 0x20);
    SET_A8(0x20);
    VramQueueTiles_m1x1();
}
void VramQueueTiles_m1x1(void)
{
    VramQueueTilesFrom(IDP(0x02));
}
/*
 * $80:A4E3 DrawRaceHud (RTS): the race HUD labels: block $80:A58B at VRAM
 * $1882, the course number ($80:A590 + course $7C * 2) at $1886, block
 * $80:A574 at $1B24 (attribute $20); in a final ($7D) also $80:A586 at $185C
 * and $80:A583 at $1859; then the player's name (roster_name) at $1852.
 */
void DrawRaceHud_m1x1(void)
{
    queue20(0x1882, 0xA594);
    push16(0xA4F3); VramQueueTiles20_m1x1();
    rep(0x20);
    lda16(0x1886); wr16(DP(0x00), 0x1886);
    lda16(rd16(DP(COURSE_SLOT)) & 0x00FF);
    lda16(asl16(A));
    fc = 0;
    adc16(0xA599);
    wr16(DP(0x02), A);
    sep(0x20);
    push16(0xA50B); VramQueueTiles20_m1x1();
    queue20(0x1B24, 0xA57D);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xA520); VramQueueTiles_m1x1();
    lda8(rd8(DP(FINAL_RACE)));
    if (!fz) {
        queue20(0x185C, 0xA58F);
        push16(0xA535); VramQueueTiles20_m1x1();
        queue20(0x1859, 0xA58C);
        push16(0xA546); VramQueueTiles20_m1x1();
    }
    rep(0x20);
    lda16(0x1852); wr16(DP(0x00), 0x1852);
    sep(0x20);
    u8 ch = rd8(ABS(PLAYER_CHAR));
    lda8(ch);
    wr8(ABS(WRMPYA), ch);                           /* WRMPYA: ch * 15 */
    lda8(0x0F);
    wr8(ABS(WRMPYB), 0x0F);
    rep(0x20);
    int st = rd8(LONG(STADIUM_SELECT)) != 0;
    lda16(rd16(ABS(RDMPYL)));                       /* RDMPYL: ch * 15 */
    fc = 0;
    adc16(0xA5A1);
    wr16(DP(0x02), A);
    lda16(rd16(LONG(STADIUM_SELECT)));
    if (!fz) {
        lda16(rd16(DP(0x02)));
        fc = 0;
        adc16(0x004B);
        wr16(DP(0x02), A);
    }
    /* the original's pointer, or the roster's own name for an added character */
    u32 name = roster_name(st, ch, rd16(DP(0x02)));
    wr16(DP(0x02), (u16)name);
    sep(0x20);
    /* JMP VramQueueTiles20 with the list at `name` */
    wr8(DP(0x0F), 0x20);
    SET_A8(0x20);
    VramQueueTilesFrom(name);
}
/*
 * $81:C0F6 WinnerFigure (RTS), result object X ($03A3): every 8 frames flip
 * its frame bit ($031F) and step it left ($027F); until it reaches its spot
 * (X < $95 in the jump contest $0309 = 4, else < $90) keep walking; there,
 * show the winner's pose: the rider in first place ($086E,rider = 1) of a
 * race of character $154F (roster_winner_pose) -> $030F, timer 64. The action
 * $034F is $81:C156[frame bit].
 */
void WinnerFigure_m1x1(void)
{
    X = rd8(ABS(OBJ_CURRENT));
    lda8(rd8(ABS(OBJ_DELAY + X)));
    if (fz) {
        lda8(rd8(ABS(OBJ_STEP + X)));
        eor8(0x01);
        wr8(ABS(OBJ_STEP + X), (u8)A);
        lda8(0x08);
        wr8(ABS(OBJ_DELAY + X), 0x08);
        wr8(ABS(OBJ_X + X), dec8(rd8(ABS(OBJ_X + X))));
        lda8(rd8(ABS(OBJ_TYPE + 0x0A)));
        cmp8((u8)A, 0x04);
        u8 limit = fz ? 0x95 : 0x90;
        lda8(rd8(ABS(OBJ_X + X)));
        cmp8((u8)A, limit);
        if (!fc) {
            ldy8(0x0E);
            for (;;) {
                lda8(dec8(rd8(ABS(PLAYER_PLACE + Y))));
                if (fz) break;
                Y = dec8((u8)Y);
                Y = dec8((u8)Y);
                if (fn) break;
            }
            lda8((u8)Y);
            lda8(lsr8((u8)A));
            wr8(DP(0x00), (u8)A);
            u8 rider = (u8)A;
            u8 ch = rd8(ABS(PLAYER_CHAR));
            lda8((u8)(ch << 3));
            fc = 0;
            adc8(rd8(DP(0x00)));
            wr8(DP(0x00), (u8)A);
            Y = (u8)A; nz8((u8)Y);
            lda8(roster_winner_pose(ch, rider));
            wr8(ABS(OBJ_STATE + X), (u8)A);
            lda8(0x40);
            wr8(ABS(OBJ_DELAY + X), 0x40);
        }
    }
    ldy8(rd8(ABS(OBJ_STEP + X)));
    lda8(rd8(ABS(0xC156 + Y)));
    wr8(ABS(OBJ_FRAME + X), (u8)A);
    ret();
}
/*
 * $80:89D5 SelectMain (RTS), every frame of the player select (mode 1).
 *   - transition in or out running ($198D): SelectTransition;
 *   - after the choice ($198F frames): keep drawing, timer >= 1;
 *   - choice made ($198B): wait for the broadcast (minute 6, frame $0708)
 *     or B to cancel; then the next segment (JMP $80:8E51 path);
 *   - choosing: turn the ring, draw; the timer running out or A (with the
 *     ring still) picks the front portrait: $154F = its character, sound
 *     buffer cleared, that character's voice samples uploaded, sound $3F.
 * Always: the countdown digits, and a new race variant $1E91 at random.
 * With the debug flag $81:800E, Start skips the select.
 */
void SelectMain_m1x1(void)
{
    lda8(rd8(LONG(DEBUG_ENABLED)));
    if (!fz) {
        lda8(rd8(DP(P1_PRESS_HI)));
        and8(0x10);
        if (!fz) goto next_segment;
    }
    lda8(rd8(ABS(SELECT_SLIDING)));
    if (!fz) {
        wr8(DP(P1_HELD_HI), 0);
        push16(0x89EA); SelectTransition_m1x1();
        goto tail;
    }
    lda8(rd8(ABS(0x198F)));
    if (!fz) {
        rep(0x20);
        wr16(ABS(MODE_TIMER), dec16(rd16(ABS(MODE_TIMER))));
        lda16(rd16(ABS(MODE_TIMER)));
        cmp16(A, 0x0001);
        if (fn) {
            lda16(0x0001);
            wr16(ABS(MODE_TIMER), 0x0001);
        }
        sep(0x20);
        wr8(ABS(0x198F), dec8(rd8(ABS(0x198F))));
        wr8(DP(P1_HELD_HI), 0);
        push8(0x80); push16(0x8A10); SelectDraw_m1x1();
        lda8(rd8(ABS(SELECT_CHOSEN)));
        if (!fz) {
            push16(0x8A18); SelectSlideY_m1x1();
            push16(0x8A1B); SelectHideRest_m1x1();
        }
        ret();
        return;
    }
    lda8(rd8(ABS(SELECT_CHOSEN)));
    if (!fz) {
        lda8(rd8(LONG(BCAST_MINUTE)));
        cmp8((u8)A, 0x06);
        if (fc) {
            if (!fz) goto next_segment;
            rep(0x20);
            lda16(rd16(LONG(BCAST_FRAME)));
            cmp16(A, 0x0708);
            sep(0x20);
            if (fc) goto next_segment;
        }
        rep(0x20);
        wr16(ABS(MODE_TIMER), dec16(rd16(ABS(MODE_TIMER))));
        sep(0x20);
        if (fz) goto next_segment;
        lda8(rd8(DP(P1_PRESS_HI)));
        and8(0x80);
        if (!fz) {
            lda8(0x3D);
            push8(0x80); push16(0x8A52); QueueSoundB_m1x1();
            lda8(0x1E);
            wr8(ABS(0x198F), 0x1E);
            wr8(ABS(SELECT_CHOSEN), 0);
            wr8(ABS(SELECT_SLIDING), inc8(rd8(ABS(SELECT_SLIDING))));
        }
        wr8(DP(P1_HELD_HI), 0);
        push8(0x80); push16(0x8A63); SelectDraw_m1x1();
        push16(0x8A66); SelectSlideY_m1x1();
        push16(0x8A69); SelectHideRest_m1x1();
        goto tail;
    }
    rep(0x20);
    lda16(rd16(ABS(MODE_TIMER)));
    cmp16(A, 0x0001);
    sep(0x20);
    if (fz)
        wr8(DP(P1_HELD_HI), 0);
    lda8(rd8(ABS(0x1E8B)));
    wr8(DP(0x1F), (u8)A);
    push8(0x80); push16(0x8A83); SelectRotate_m1x1();
    push8(0x80); push16(0x8A87); SelectDraw_m1x1();
    rep(0x20);
    wr16(ABS(MODE_TIMER), dec16(rd16(ABS(MODE_TIMER))));
    sep(0x20);
    if (fz) {
        lda8(rd8(ABS(OAM_BUF + 0x101)));
        cmp8((u8)A, 0xF0);
        if (fz) {
            lda8(0x01);
            wr8(ABS(MODE_TIMER), 0x01);
            goto tail;
        }
    } else {
        lda8(rd8(DP(P1_PRESS_LO)));
        and8(0x80);
        if (fz) goto tail;
        lda8(rd8(ABS(SELECT_TURN)));
        if (!fz) goto tail;
    }
    /* the choice */
    wr8(DP(SCROLL_HALF), inc8(rd8(DP(SCROLL_HALF))));
    wr8(ABS(SELECT_CHOSEN), inc8(rd8(ABS(SELECT_CHOSEN))));
    lda8(0x15);
    wr8(ABS(SH_TM), 0x15);
    wr8(ABS(SELECT_SLIDING), inc8(rd8(ABS(SELECT_SLIDING))));
    lda8(0x1E);
    wr8(ABS(0x198F), 0x1E);
    {
        X = rd8(ABS(0x1579));
        u8 ch = roster_select_char(X);
        u8 ech = roster_engine_char(ch);
        lda8(ech);
        wr8(ABS(PLAYER_CHAR), ech);
        roster_note_choice(ch);
        wr8(ABS(NMITIMEN), 0);
        rep(0x20);
        for (int x = 0x3E; x >= 0; x -= 2) {
            wr16(ABS(SND_RING + x), 0);
            wr16(ABS(SND_RING + 0x40 + x), 0);
            wr16(ABS(SND_RING + 0x80 + x), 0);
            wr16(ABS(SND_RING + 0xC0 + x), 0);
        }
        X = 0xFE;
        sep(0x20);
        static const u8 cleared[] = { 0x69, 0x65, 0x6D, 0x6A, 0x66, 0x6E, 0x6B, 0x67, 0x6F, 0x6C, 0x68, 0x70 };
        for (unsigned k = 0; k < sizeof cleared; k++)
            wr8(DP(cleared[k]), 0);
        u32 stream = roster_voice_stream(ch);
        X = rd8(ABS(PLAYER_CHAR));
        X = inc8((u8)X);
        if (stream) {
            SpcUploadFrom(stream);
        } else {
            push8(0x80); push16(0x8B01); SpcUploadB_m1x1();
        }
        lda8(0x3F);
        push8(0x80); push16(0x8B0B); QueueSoundB_m1x1();
        lda8(rd8(ABS(SH_NMITIMEN)));
        wr8(ABS(NMITIMEN), (u8)A);
    }
tail:
    push16(0x8B14); SelectTimer_m1x1();
    push8(0x80); push16(0x8B18); RandomB_m1x1();
    and8(0x01);
    wr8(ABS(RACE_VARIANT), (u8)A);
    ret();
    return;

next_segment:
    /* $80:8E51: course and final flag from the broadcast minute, then the
     * course map (mode $18) or the jump contest (mode $17); a final
     * continues at $80:8F15. */
    {
        ldx8(0xFF);
        wr8(DP(0x00), 0xFF);
        lda8(rd8(LONG(BCAST_MINUTE)));
        do {
            wr8(DP(0x00), inc8(rd8(DP(0x00))));
            X = inc8((u8)X);
            cmp8((u8)A, rd8(LONG(0x808E46 + X)));
            WAIT_TICK();
        } while (fc);
        cmp8((u8)X, 0x04);
        if (fz) {
            lda8(0x17);
            wr8(DP(GAME_MODE), 0x17);
            r816_call_pc(0x808F15);
            return;
        }
        if (!fc)
            ;
        else
            wr8(DP(0x00), dec8(rd8(DP(0x00))));
        lda8(rd8(DP(0x00)));
        lda8(lsr8((u8)A));
        wr8(DP(COURSE_SLOT), (u8)A);
        lda8(rol8((u8)A));
        and8(0x01);
        wr8(DP(FINAL_RACE), (u8)A);
        if (!fz) {
            r816_call_pc(0x808F13);
            return;
        }
        lda8(0x18);
        wr8(DP(GAME_MODE), 0x18);
        r816_call_pc(0x808F15);
    }
}
/*
 * $80:8B24 SelectTimer (RTS): the countdown in seconds (timer $88 / 60 with
 * the hardware divider) as two digits at VRAM $1A8F (list at $03CF: tiles
 * $00-$09 on top, $10-$19 below).
 */
void SelectTimer_m1x1(void)
{
    lda8(rd8(DP(MODE_TIMER))); wr8(ABS(WRDIVL), (u8)A);
    lda8(rd8(DP(0x89))); wr8(ABS(WRDIVH), (u8)A);
    lda8(0x3C); wr8(ABS(WRDIVB), 0x3C);
    wr8(DP(0x00), 0);
    lda8(rd8(ABS(RDDIVL)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        wr8(DP(0x00), inc8(rd8(DP(0x00))));
        WAIT_TICK();
    }
    wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
    ora8(0x10);
    wr8(ABS(DIGIT_ROW + 0x03), (u8)A);
    lda8(rd8(DP(0x00)));
    wr8(ABS(DIGIT_ROW), (u8)A);
    ora8(0x10);
    wr8(ABS(DIGIT_ROW + 0x02), (u8)A);
    lda8(0x22);
    wr8(ABS(DIGIT_BUF), 0x22);
    rep(0x20);
    lda16(0x1A8F); wr16(DP(0x00), 0x1A8F);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    push16(0x8B71); VramQueueTiles20_m1x1();
    ret();
}
/*
 * $80:8B73 SelectRotate (RTL): start turning when left or right is pressed
 * ($1559 = $80:8BAA[pad]: +2 / -2, $198C = the direction); while turning,
 * a tick sound every 4 frames; turn every portrait and place it.
 */
void SelectRotate_m1x1(void)
{
    lda8(rd8(ABS(SELECT_TURN)));
    int tick = 0;
    if (!fz) {
        tick = 1;
    } else {
        lda8(rd8(DP(P1_HELD_HI)));
        and8(0x03);
        if (!fz) {
            wr8(ABS(0x198C), (u8)A);
            X = (u8)A; nz8((u8)X);
            lda8(rd8(ABS(0x8BA8 + X)));
            wr8(ABS(SELECT_TURN), (u8)A);
            tick = 1;
        }
    }
    if (tick) {
        lda8(rd8(DP(FRAME_COUNT)));
        and8(0x03);
        if (fz) {
            lda8(0x40);
            push8(0x80); push16(0x8B93); QueueSoundB_m1x1();
        }
    }
    ldx8(0x04);
    do {
        lda8(rd8(ABS(0x1554 + X)));
        fc = 0;
        adc8(rd8(ABS(SELECT_TURN)));
        wr8(ABS(0x1554 + X), (u8)A);
        push8(0x80); push16(0x8BA3); CarouselPos_m1x1();
        X = dec8((u8)X);
    } while (!fn);
    retl();
}
/*
 * $80:8BAC SelectDraw (RTL): which portrait is in front ($1579, from the
 * ring angle) and every portrait as a 3x3 sprite block, front first. The
 * front one, when the ring is still, gets its bright palette and its name;
 * the others are dimmed (palette 4).
 */
void SelectDraw_m1x1(void)
{
    u8 a0 = rd8(ABS(0x1554));
    lda8(a0);
    u8 cursor = roster_select_cursor(a0);
    lda8(cursor);
    Y = cursor;
    wr8(ABS(0x1579), cursor);
    int st = rd8(LONG(STADIUM_SELECT)) != 0;
    for (int i = roster_count() - 1; i >= 5; i--)
        draw_portrait(i, cursor, st);
    for (int i = 4; i >= 0; i--) {
        X = (u8)i; nz8((u8)X);
        draw_portrait(i, cursor, st);
    }
    X = 0xFF; nz8(0xFF);
    retl();
}
/* $80:8D26 SelectHideRest (RTS): hide every portrait sprite but the front
 * portrait's (Y = $F0). */
void SelectHideRest_m1x1(void)
{
    ldx8(0x25);
    lda8(0xF0);
    u8 end = (u8)(1 + 36 * roster_count());          /* $B5 for five portraits */
    do {
        wr8(ABS(OAM_BUF + X), 0xF0);
        X = (u8)(X + 4);
        cmp8((u8)X, end);
    } while (!fz);
    ret();
}
/* $80:8D36 SelectSlideY (RTS): move the front portrait to row $198E (and
 * its name 4 pixels below it). */
void SelectSlideY_m1x1(void)
{
    ldx8(0x00);
    lda8(rd8(ABS(SELECT_ROW)));
    do {
        wr8(ABS(OAM_BUF + 0x01 + X), (u8)A);
        wr8(ABS(OAM_BUF + 0x05 + X), (u8)A);
        wr8(ABS(OAM_BUF + 0x09 + X), (u8)A);
        fc = 0;
        adc8(0x10);
        push8((u8)A);
        lda8((u8)X); fc = 0; adc8(0x0C);
        X = (u8)A;
        lda8(pull8());
        cmp8((u8)X, 0x24);
    } while (!fz);
    fc = 0;
    adc8(0x04);
    for (int k = 0; k < 6; k++)
        wr8(ABS(OAM_BUF + 0x101 + k * 4), (u8)A);
    ret();
}
/*
 * $80:8D6A SelectTransition (RTS): after the choice ($198B = 1) or on the
 * way in, count the timer $88 down (three per frame when leaving), slide the
 * front portrait (one row per frame towards $80:8D6A[$198B]) and hide the
 * others; at the target row the transition ends ($198D = 0; on the way in
 * also $90 = 0 and main screen $17).
 */
void SelectTransition_m1x1(void)
{
    rep(0x20);
    wr16(ABS(MODE_TIMER), dec16(rd16(ABS(MODE_TIMER))));
    X = rd8(ABS(SELECT_CHOSEN)); nz8((u8)X);
    if (!fz) {
        wr16(ABS(MODE_TIMER), dec16(rd16(ABS(MODE_TIMER))));
        wr16(ABS(MODE_TIMER), dec16(rd16(ABS(MODE_TIMER))));
    }
    lda16(rd16(ABS(MODE_TIMER)));
    if (fn) {
        lda16(0x0001);
        wr16(ABS(MODE_TIMER), 0x0001);
    }
    sep(0x20);
    push8(0x80); push16(0x8D8A); SelectDraw_m1x1();
    push16(0x8D8D); SelectSlideY_m1x1();
    push16(0x8D90); SelectHideRest_m1x1();
    X = rd8(ABS(SELECT_CHOSEN));
    cmp8((u8)X, 0x01);
    if (fz)
        wr8(ABS(SELECT_ROW), dec8(rd8(ABS(SELECT_ROW))));
    else
        wr8(ABS(SELECT_ROW), inc8(rd8(ABS(SELECT_ROW))));
    lda8(rd8(ABS(SELECT_ROW)));
    cmp8((u8)A, rd8(ABS(0x8D68 + X)));
    if (fz) {
        lda8(rd8(ABS(SELECT_CHOSEN)));
        if (fz) {
            wr8(DP(SCROLL_HALF), 0);
            lda8(0x17);
            wr8(ABS(SH_TM), 0x17);
        }
        wr8(ABS(SELECT_SLIDING), 0);
    }
    ret();
}
/*
 * $80:8DB8 SelectName (RTL), portrait X in front: its name as six 8x8
 * sprites at $1700 (x $68-$90, y $84): tiles row * 16 + 6.. (row from the
 * roster: the portrait, or the Stadium's own for the shared slot),
 * attribute $21, small.
 */
void SelectName_m1x1(void)
{
    lda8(0x68);
    for (int k = 0; k < 6; k++) {
        wr8(ABS(OAM_BUF + 0x100 + k * 4), (u8)A);
        if (k < 5) { fc = 0; adc8(0x08); }
    }
    lda8(0x84);
    for (int k = 0; k < 6; k++)
        wr8(ABS(OAM_BUF + 0x101 + k * 4), 0x84);
    lda8((u8)X);
    cmp8((u8)X, 0x03);
    int st = rd8(LONG(STADIUM_SELECT)) != 0;
    u8 row = roster_select_name_row(st, X);
    if (fz) {
        push8((u8)X);
        lda8(rd8(LONG(STADIUM_SELECT)));
        X = (u8)A;
        lda8(rd8(LONG(0x808E44 + X)));
        X = pull8();
    }
    lda8(row);
    for (int i = 0; i < 4; i++)
        lda8(asl8((u8)A));
    fc = 0;
    adc8(0x06);
    for (int k = 0; k < 6; k++) {
        wr8(ABS(OAM_BUF + 0x102 + k * 4), (u8)A);
        if (k < 5) lda8(inc8((u8)A));
    }
    lda8(0x21);
    for (int k = 0; k < 6; k++)
        wr8(ABS(OAM_BUF + 0x103 + k * 4), 0x21);
    for (int k = 0; k < 6; k++)
        wr8(ABS(OAM_SIZE_BUF + 0x40 + k), 0);
    retl();
}
/*
 * $81:A24B CarouselPos (RTL), portrait X: its screen position from its angle
 * (x = $68 + sin part, y = $50 + cos part, radius $40) into $155A,X /
 * $155F,X. For portrait 0, when the ring reaches a stop angle while turning
 * and the d-pad is no longer held the same way, stop ($1559 = 0, sound $41).
 */
void CarouselPos_m1x1(void)
{
    push8((u8)X);
    lda8(0x40);
    wr8(DP(0x08), 0x40);
    lda8(rd8(ABS(0x1554 + X)));
    X = (u8)A; nz8((u8)X);
    push8(0x81); push16(0xA257); SinCos_m1x1();
    X = pull8(); nz8((u8)X);
    lda8(0x68); fc = 0; adc8(rd8(DP(0x04)));
    wr8(ABS(0x155A + X), (u8)A);
    lda8(0x50); fc = 0; adc8(rd8(DP(0x00)));
    wr8(ABS(0x155F + X), (u8)A);
    cmp8((u8)X, 0x00);
    if (fz) {
        u8 a0 = rd8(ABS(0x1554));
        lda8(a0);
        if (roster_select_is_stop(a0)) {
            lda8(rd8(ABS(SELECT_TURN)));
            if (!fz) {
                lda8(rd8(DP(P1_HELD_HI)));
                and8(0x03);
                cmp8((u8)A, rd8(ABS(0x198C)));
                if (!fz) {
                    lda8(0x41);
                    push8(0x81); push16(0xA297); QueueSoundB_m1x1();
                    wr8(ABS(SELECT_TURN), 0);
                }
            }
        }
    }
    retl();
}
/*
 * $81:A29C SinCos (RTL), X = angle: $00 = y offset, $04 = x offset for the
 * radius in $08: y = radius * cos >> 8 / 4 (the ellipse is flat), x = radius
 * * sin >> 8; signs from $81:A527 / $81:A627, magnitudes from $81:A427 /
 * $81:A327 (multiplied with the hardware multiplier, rounded).
 */
void SinCos_m1x1(void)
{
    push8((u8)X);
    lda8(rd8(DP(0x08)));
    wr8(ABS(WRMPYA), (u8)A);
    lda8(rd8(LONG(0x81A427 + X)));
    wr8(ABS(WRMPYB), (u8)A);
    lda8(rd8(LONG(0x81A527 + X)));
    wr8(DP(0x02), (u8)A);
    wr8(DP(0x03), 0);
    lda8(rd8(ABS(RDMPYL)));
    lda8(asl8((u8)A));
    lda8(rd8(ABS(RDMPYH)));
    adc8(0x00);
    wr8(DP(0x00), (u8)A);
    wr8(DP(0x01), 0);
    lda8(rd8(LONG(0x81A327 + X)));
    wr8(ABS(WRMPYB), (u8)A);
    lda8(rd8(LONG(0x81A627 + X)));
    wr8(DP(0x06), (u8)A);
    wr8(DP(0x07), 0);
    lda8(rd8(ABS(RDMPYL)));
    lda8(asl8((u8)A));
    lda8(rd8(ABS(RDMPYH)));
    adc8(0x00);
    wr8(DP(0x04), (u8)A);
    wr8(DP(0x05), 0);
    X = pull8();
    rep(0x20);
    lda16(rd16(DP(0x00)));
    if (!fn) {
        lda16(lsr16(A));
        lda16(lsr16(A));
    } else {
        fc = 1;
        lda16(ror16(A));
        lda16(ror16(A));
        ora16(0x8000);
    }
    ldy8(rd8(DP(0x02)));
    if (fz) {
        eor16(0xFFFF);
        lda16(inc16(A));
    }
    wr16(DP(0x00), A);
    lda16(rd16(DP(0x04)));
    ldy8(rd8(DP(0x06)));
    if (!fz) {
        eor16(0xFFFF);
        lda16(inc16(A));
    }
    wr16(DP(0x04), A);
    sep(0x20);
    retl();
}
void QueueSoundB_m1x1(void) { queue_sound_body(); }
void QueueSoundB_m0x1(void) { queue_sound_body(); }
void RandomB_m1x1(void) { random_body(); }
void RandomB_m1x0(void) { random_body(); }
/* $82:8593 SpcUploadB (RTL), X = stream number: $82:857E/8585/858C[X]. */
void SpcUploadB_m1x1(void)
{
    lda8(0xFF);
    wr8(ABS(APUIO0), 0xFF);
    lda8(rd8(LONG(0x82857E + X))); wr8(DP(0x00), (u8)A);
    lda8(rd8(LONG(0x828585 + X))); wr8(DP(0x01), (u8)A);
    lda8(rd8(LONG(0x82858C + X))); wr8(DP(0x02), (u8)A);
    upload_from_dp();
    retl();
}
/*
 * $80:81A9 DrawClockB (RTS): while the clock is on ($1566), every 30 frames
 * blink the colon ($1E85) and queue "HH:MM" as a 5x2 tile block at VRAM
 * $1842 (list $1E1E: size $25, digits $00-$09 on top and $10-$19 below,
 * colon $46/$56 or blank $FF; attribute $20, or $24 with $CA set while the
 * colon is off). $1566 negative forces a redraw now. In the port the digits
 * are drawn blank when the game plays without the broadcast clock.
 */
void DrawClockB_m1x1(void)
{
    lda8(rd8(ABS(0x1566)));
    if (!fn) {
        if (fz) { ret(); return; }
        u8 t = dec8(rd8(ABS(0x1E84)));
        wr8(ABS(0x1E84), t);
        if (t != 0) { ret(); return; }
    }
    lda8(0x01); wr8(ABS(0x1566), 0x01);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    lda8(rd8(ABS(0x1E85)));
    eor8(0x01);
    wr8(ABS(0x1E85), (u8)A);
    lda8(0x1E); wr8(ABS(0x1E84), 0x1E);
    lda8(0x42); wr8(DP(0x00), 0x42);
    lda8(0x18); wr8(DP(0x01), 0x18);
    lda8(0x25); wr8(ABS(0x1E1E), 0x25);
    u8 u = split_tens(rd8(LONG(BCAST_HOUR)), 0x1E1F);       /* hour */
    wr8(ABS(0x1E20), u);
    ora8(0x10); wr8(ABS(0x1E25), (u8)A);
    lda8(rd8(ABS(0x1E1F))); fc = 0; adc8(0x10); wr8(ABS(0x1E24), (u8)A);
    u = split_tens(rd8(LONG(BCAST_MINUTE)), 0x1E22);           /* minute */
    wr8(ABS(0x1E23), u);
    ora8(0x10); wr8(ABS(0x1E28), (u8)A);
    lda8(rd8(ABS(0x1E22))); fc = 0; adc8(0x10); wr8(ABS(0x1E27), (u8)A);
    lda8(rd8(ABS(0x1E85)));
    if (fz) {
        lda8(0x46); wr8(ABS(0x1E21), 0x46);
        lda8(0x56); wr8(ABS(0x1E26), 0x56);
    } else {
        lda8(0xFF);
        wr8(ABS(0x1E21), 0xFF);
        wr8(ABS(0x1E26), 0xFF);
        lda8(rd8(ABS(0x00CA)));
        if (!fz) { lda8(0x24); wr8(DP(0x0F), 0x24); }
    }
    if (port_hide_clock())
        for (int a = 0x1E1F; a <= 0x1E28; a++)
            wr8(ABS(a), 0xFF);
    lda8(0x1E); wr8(DP(0x02), 0x1E);
    lda8(0x1E); wr8(DP(0x03), 0x1E);
    VramQueueTiles_m1x1();                            /* JMP: it returns for us */
}
/* $80:C992 Reach20 (RTS): A + $14 unless A >= $E7 (then A unchanged). */
void Reach20_m1x1(void)
{
    cmp8((u8)A, 0xE7);
    if (!fc)
        adc8(0x14);
    ret();
}
/* $80:C999 Reach2 (RTS): A + 2 unless A >= $E7. */
void Reach2_m1x1(void)
{
    cmp8((u8)A, 0xE7);
    if (!fc)
        adc8(0x02);
    ret();
}
/*
 * $81:C6F9 CoinMilestone (RTL): at exactly 100 or 200 coins ($1542 = $64 /
 * $C8) start the bonus display: $199D = the count, $09AD = $199A = 1,
 * $199B = $40, $199C = the player's Y - 16, and sound $2B.
 */
void CoinMilestone_m1x1(void)
{
    lda8(rd8(ABS(COIN_TOTAL)));
    cmp8((u8)A, 0x64);
    int hit = fz;
    if (!hit) {
        cmp8((u8)A, 0xC8);
        hit = fz;
    }
    if (hit) {
        wr8(ABS(0x199D), (u8)A);
        lda8(0x01);
        wr8(ABS(SCORE_ADD + 0x01), 0x01);
        wr8(ABS(0x199A), 0x01);
        lda8(0x40);
        wr8(ABS(0x199B), 0x40);
        lda8(rd8(ABS(RIDER_Y)));
        fc = 1;
        sbc8(0x10);
        wr8(ABS(0x199C), (u8)A);
        lda8(0x2B);
        push8(0x81); push16(0xC722); QueueSoundB_m1x1();
    }
    retl();
}
/*
 * $80:D093 CollectCoin (RTL): sound $31; take a free coin effect slot
 * ($1530,X = 0, X = 0, 2, 4, 6) or, with all four busy, the oldest
 * ($1538,X largest, counted in $1541) and start it at the player ($01B6 + 16;
 * speed $F8/$E0, timer $38); count the coin ($1542, milestone check), the
 * combo $1540 (up to 5) adds $80:D11D/$80:D124[combo] to the score digits
 * $09AF/$09B0, and $1543/$1543,combo count up.
 */
void CollectCoin_m1x1(void)
{
    lda8(0x31);
    push8(0x80); push16(0xD098); QueueSoundB_m1x1();
    wr8(DP(0x00), 0);
    wr8(DP(0x01), 0);
    ldx8(0x00);
    for (;;) {
        rep(0x20);
        lda16(rd16(ABS(MARKER_AGE + X)));
        cmp16(A, rd16(DP(0x00)));
        if (fc) {
            wr16(DP(0x00), A);
            wr8(DP(0x02), (u8)X);
        }
        sep(0x20);
        lda8(rd8(ABS(MARKER_ON + X)));
        if (fz)
            break;
        X = (u8)(X + 2);
        cmp8((u8)X, 0x08);
        if (fc) {
            ldx8(rd8(DP(0x02)));
            wr8(ABS(MARKER_COUNT), inc8(rd8(ABS(MARKER_COUNT))));
            break;
        }
    }
    wr8(ABS(MARKER_AGE + X), 0);
    wr8(ABS(MARKER_AGE + 0x01 + X), 0);
    lda8(0x01);
    wr8(ABS(MARKER_ON + X), 0x01);
    rep(0x20);
    lda16(0x0038); wr16(ABS(MARKER_X + X), 0x0038);
    lda16(rd16(ABS(RIDER_Y)));
    fc = 0;
    adc16(0x0010);
    wr16(ABS(MARKER_Y + X), A);
    lda16(0x00F8); wr16(ABS(MARKER_VX + X), 0x00F8);
    lda16(0x00E0); wr16(ABS(MARKER_VY + X), 0x00E0);
    wr16(ABS(MARKER_FRAC + X), 0);
    wr16(ABS(0x1528 + X), 0);
    sep(0x20);
    wr8(ABS(COIN_TOTAL), inc8(rd8(ABS(COIN_TOTAL))));
    push8(0x80); push16(0xD0F5); CoinMilestone_m1x1();
    lda8(rd8(ABS(COIN_COMBO)));
    cmp8((u8)A, 0x05);
    if (fc) {
        ldy8(0x05);
    } else {
        wr8(ABS(COIN_COMBO), inc8(rd8(ABS(COIN_COMBO))));
        ldy8(rd8(ABS(COIN_COMBO)));
    }
    lda8(rd8(ABS(0xD126 + Y)));
    fc = 0;
    adc8(rd8(ABS(SCORE_ADD + 0x03)));
    wr8(ABS(SCORE_ADD + 0x03), (u8)A);
    lda8(rd8(ABS(0xD12D + Y)));
    fc = 0;
    adc8(rd8(ABS(SCORE_ADD + 0x04)));
    wr8(ABS(SCORE_ADD + 0x04), (u8)A);
    wr8(ABS(0x1543), inc8(rd8(ABS(0x1543))));
    lda8(rd8(ABS(0x1543 + Y)));
    lda8(inc8((u8)A));
    wr8(ABS(0x1543 + Y), (u8)A);
    retl();
}
/*
 * $80:C77F RiderCollisions (RTS): every pair of riders X < Y that are both
 * racing ($01DE), on the ground ($019E = 1), in the same lane band ($0958,
 * non-zero), not already bumped ($01CE; X also not in $0940) and close
 * enough (height $01B6 within $14 when either is in the air $01EE; X
 * position $0196 within 2 for a rear bump or $14 for a side bump):
 *   - rear bump: rider X falls back: crash sound, $01CE,X = $FF (and sound
 *     $3D for the player);
 *   - side bump: both bump; for the player's first bump in a while with coins
 *     out ($01BE/$01C6) a coin is won (CollectCoin, $0CDF counts to 5:
 *     sounds $2D and $01, $0CE0 = 1, $0990 = $20); rider Y gets $01CE = $FF,
 *     its crash voice ($80:C8E9 by character, Stadium, race variant and
 *     rider) and the crash sound.
 * $5F = $10 when the player and rider 7 meet.
 */
void RiderCollisions_m1x1(void)
{
    ldx8(0x00);
    ldy8(0x01);
next_x_check:
    lda8(rd8(ABS(RIDER_ACTIVE + X)));
    if (fz) goto next_x;
pair:
    lda8(rd8(ABS(RIDER_ACTIVE + Y)));
    if (fz) goto next_y;
    lda8(rd8(ABS(RIDER_PAGE + X))); cmp8((u8)A, 0x01);
    if (!fz) goto next_x;
    lda8(rd8(ABS(RIDER_PAGE + Y))); cmp8((u8)A, 0x01);
    if (!fz) goto next_y;
    lda8(rd8(ABS(RIDER_BAND + X)));
    if (fz) goto next_x;
    cmp8((u8)A, rd8(ABS(RIDER_BAND + Y)));
    if (!fz) goto next_y;
    lda8(rd8(ABS(RIDER_BUMPED + X)));
    ora8(rd8(ABS(OVERHEAT_LOCK + X)));
    if (!fz) goto next_x;
    lda8(rd8(ABS(RIDER_BUMPED + Y)));
    if (!fz) goto next_y;
    lda8(rd8(ABS(RIDER_AIR + X)));
    if (fz) {
        lda8(rd8(ABS(RIDER_AIR + Y)));
        if (fz) goto side;
    }
    lda8(rd8(ABS(RIDER_Y + X)));
    cmp8((u8)A, rd8(ABS(RIDER_Y + Y)));
    if (!fc) {
        push16(0xC7C9); Reach20_m1x1();
        cmp8((u8)A, rd8(ABS(RIDER_Y + Y)));
        if (fc) goto side;
        goto next_y;
    }
    lda8(rd8(ABS(RIDER_Y + Y)));
    push16(0xC7D7); Reach20_m1x1();
    cmp8((u8)A, rd8(ABS(RIDER_Y + X)));
    if (!fc) goto next_y;
side:
    lda8(rd8(ABS(RIDER_X + X)));
    cmp8((u8)A, rd8(ABS(RIDER_X + Y)));
    if (!fc) {
        push16(0xC7E7); Reach2_m1x1();
        cmp8((u8)A, rd8(ABS(RIDER_X + Y)));
        if (fc) goto bump_both;
        lda8(rd8(ABS(RIDER_X + X)));
        push16(0xC7F2); Reach20_m1x1();
        cmp8((u8)A, rd8(ABS(RIDER_X + Y)));
        if (fc) goto rear_bump;
        goto next_y;
    }
    lda8(rd8(ABS(RIDER_X + Y)));
    push16(0xC7FF); Reach2_m1x1();
    cmp8((u8)A, rd8(ABS(RIDER_X + X)));
    if (fc) goto bump_both;
    lda8(rd8(ABS(RIDER_X + Y)));
    push16(0xC80A); Reach20_m1x1();
    cmp8((u8)A, rd8(ABS(RIDER_X + X)));
    if (fc) goto meet;
    goto next_y;

rear_bump:
    crash_sound((u8)X, 0xC81E);
    cmp8((u8)X, 0x00);
    if (fz) sound80(0x3D, 0xC828);
    lda8(0xFF);
    wr8(ABS(RIDER_BUMPED + X), 0xFF);
    goto after_bump;

bump_both:
    crash_sound((u8)X, 0xC83C);
    lda8(0xFF);
    wr8(ABS(RIDER_BUMPED + X), 0xFF);
meet:
    cmp8((u8)X, 0x00);
    if (fz) {
        cmp8((u8)Y, 0x07);
        if (fz) { lda8(0x10); wr8(DP(0x5F), 0x10); }
    }
    lda8(rd8(ABS(RIDER_BUMPED)));
    if (fz) {
        cmp8((u8)X, 0x00);
        if (fz) {
            lda8(rd8(ABS(RIDER_SPEED_LO)));
            ora8(rd8(ABS(RIDER_SPEED_HI)));
            if (!fz) {
                wr8(ABS(COIN_COUNT), inc8(rd8(ABS(COIN_COUNT))));
                sound80(0x29, 0xC867);
                push8((u8)Y);
                push8((u8)X);
                push8(0x80); push16(0xC86D); CollectCoin_m1x1();
                X = pull8();
                Y = pull8(); nz8((u8)Y);
                lda8(rd8(ABS(COIN_COUNT)));
                cmp8((u8)A, 0x05);
                if (fz) {
                    sound80(0x2D, 0xC885);
                    lda8(0x01);
                    wr8(ABS(TURBO_ON), 0x01);
                    push8(0x80); push16(0xC88E); QueueSoundB_m1x1();
                    lda8(0x20);
                    wr8(ABS(ENGINE_HEAT), 0x20);
                } else if (fc) {
                    lda8(0x06);
                    wr8(ABS(COIN_COUNT), 0x06);
                }
            }
        }
    }
    lda8(0xFF);
    wr8(ABS(RIDER_BUMPED + Y), 0xFF);
    push8((u8)Y);
    u8 rider = (u8)Y;
    wr8(ABS(0x000F), (u8)Y);
    lda8(rd8(ABS(PLAYER_CHAR)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    fc = 0;
    adc8(rd8(DP(0x0F)));
    wr8(DP(0x0F), (u8)A);
    lda8(rd8(LONG(STADIUM_SELECT)));
    if (!fz) {
        lda8(0x50);
        fc = 0;
        adc8(rd8(DP(0x0F)));
        wr8(DP(0x0F), (u8)A);
    }
    lda8(rd8(ABS(RACE_VARIANT)));
    if (!fz) lda8(0x28);
    fc = 0;
    adc8(rd8(DP(0x0F)));
    Y = (u8)A; nz8((u8)Y);
    /* crash voice $80:C8E9,Y (the roster's for an added character's race) */
    lda8(roster_crash_voice(rd8(LONG(STADIUM_SELECT)) != 0, rd8(ABS(RACE_VARIANT)) != 0,
                            rd8(ABS(PLAYER_CHAR)), rider));
    if (!fz) {
        push8(0x80); push16(0xC8C8); QueueSoundB_m1x1();
    }
    Y = pull8();
    crash_sound((u8)Y, 0xC8D5);
after_bump:
    cmp8((u8)X, 0x00);
    if (fz) lda8(0x01);
next_y:
    Y = inc8((u8)Y);
    cmp8((u8)Y, 0x08);
    if (!fz) goto pair;
next_x:
    X = inc8((u8)X);
    lda8((u8)X);
    Y = (u8)A;
    Y = inc8((u8)Y);
    cmp8((u8)X, 0x07);
    if (!fz) goto next_x_check;
    ret();
}
/*
 * $82:8232 DmaFillZero (M=0, X=1, RTL). Clears ARG_LEN bytes at Y:A.
 * Sets the Mode 7 multiplier to 1 x 0 so MPYL ($2134) reads zero, then DMAs
 * from that register (B->A, fixed B address) into memory.
 */
void DmaFillZero_m0x1(void)
{
    u16 dst = A;
    u8 bank = (u8)Y;
    wr16(ARG_SRC, 0);                        /* also used as the zero for M7B */
    wr16(ABS(REG_A1T0L), dst);
    wr8(ABS(REG_A1B0), bank);
    wr8(ABS(REG_M7A), 0x01);                 /* M7A = $0001 (two writes) */
    wr8(ABS(REG_M7A), 0x00);
    wr8(ABS(REG_M7B), rd8(ARG_SRC));         /* x 0 */
    wr16(ABS(REG_DMAP0), 0x3480);            /* DMAP0 = $80 (B->A), BBAD0 = $34 (MPYL) */
    wr16(ABS(REG_DAS0L), rd16(ARG_LEN));
    wr8(ABS(REG_MDMAEN), 0x01);
    A = rd16(ARG_LEN);
    Y = 0x01;
    retl();
}
/*
 * $82:8000 ReadJoypads (M=1, X=1, RTL). Waits for auto-joypad read to finish,
 * then reads both pads into the direct page joypad block.
 */
void ReadJoypads_m1x1(void)
{
    while (rd8(ABS(REG_HVBJOY)) & 1)
        WAIT_TICK();
    fc = 0;                                 /* the wait loop's LSR leaves C = bit 0 = 0 */
    read_pad(REG_JOY1L, 0x24, 0x26, 0x2A, 0x20, 0x21, 0x28);
    read_pad(REG_JOY2L, 0x25, 0x27, 0x2B, 0x22, 0x23, 0x29);
    retl();
}
/*
 * $80:8535 NmiFlushVramQueue (M=1, X=1, RTL), in vblank. DMAs every queued
 * record to VRAM with channel 0, then empties the queue (head = $0002).
 * Head $FFFF means the queue is being written: skip this frame.
 */
void NmiFlushVramQueue_m1x1(void)
{
    push8(DB);
    push8(0x80); DB = pull8();                         /* PHK / PLB */
    rep(0x30);
    Y = 0;
    wr16(DP(0x02), 0);
    sep(0x20);
    X = 0;
    push8(DB);
    push8(0x7F); DB = pull8();                         /* queue bank */
    wr8(DP(0x00), 0x7F);
    rep(0x20);
    u16 head = rd16(ABS(0x0000));
    A = head;
    wr16(DP(0x04), head);
    int locked = head == 0xFFFF;
    int empty = rd16(DP(0x02)) == 0 && head == 0x0002;   /* $02 was just cleared */
    if (!locked && !empty) {
        u16 x = 2;
        do {
            wr16(LONG(DMAP0), rd16(ABS(x + 3)));    /* DMAP0, BBAD0 */
            wr16(LONG(VMADDL), rd16(ABS(x + 0)));    /* VMADD */
            wr16(LONG(A1T0L), rd16(ABS(x + 5)));    /* A1T0 */
            wr16(LONG(A1B0), rd16(ABS(x + 7)));    /* A1B0 (and DAS0L, rewritten next) */
            wr16(LONG(DAS0L), rd16(ABS(x + 8)));    /* DAS0 */
            sep(0x20);
            wr8(LONG(VMAIN), rd8(ABS(x + 2)));      /* VMAIN */
            wr8(LONG(MDMAEN), 0x01);                 /* MDMAEN: go */
            rep(0x20);
            x = rd16(ABS(x + 0x0A));                   /* next record */
            A = x;
        } while (x != rd16(DP(0x04)));
        X = x;
        A = 0x0002;
        wr16(ABS(0x0000), 0x0002);                     /* queue empty */
    }
    DB = pull8();
    sep(0x30);
    DB = pull8();
    retl();
}
/*
 * $80:B4D7 StepLfsr (M=1, X=1, RTS). One step of the 96-bit shift register
 * in $2C-$37 (a random source): the new top bit is bit 1 of $2C XOR bit 1 of
 * $2D, shifted in from the left across all twelve bytes. $0000 is used as
 * scratch (absolute, so it lands at DB:0000).
 */
void StepLfsr_m1x1(void)
{
    u8 a = rd8(DP(RANDOM_LFSR)) & 0x02;
    wr8(ABS(0x0000), a);
    a = (u8)((rd8(DP(0x2D)) & 0x02) ^ rd8(ABS(0x0000)));
    lda8(a);
    fc = a != 0;
    for (int i = 0x2C; i <= 0x37; i++)
        wr8(DP(i), ror8(rd8(DP(i))));
    ret();
}
/*
 * $80:8603 Mode00_IntroWait (M=1, X=1, RTS). Waits for the race segment
 * (minute 4) or a timer ($88) before switching to the title (mode $0E, step 1).
 * With the debug flag $81:800E set, Start skips the wait.
 */
void Mode00_IntroWait_m1x1(void)
{
    int skip = 0;
    lda8(rd8(LONG(DEBUG_ENABLED)));
    if ((u8)A != 0) {
        lda8(rd8(DP(P1_PRESS_HI)));
        and8(0x10);
        skip = (u8)A != 0;
    }
    if (!skip) {
        lda8(rd8(LONG(BCAST_MINUTE)));
        cmp8((u8)A, 0x04);
        if (!fc) {
            rep(0x20);
            wr16(DP(MODE_TIMER), dec16(rd16(DP(MODE_TIMER))));
            sep(0x20);
            if (!fz) { ret(); return; }
        }
    }
    lda8(0x0E);
    wr8(DP(GAME_MODE), 0x0E);
    wr8(DP(MODE_ACTIVE), 0);
    lda8(0x01);
    wr8(DP(MODE_STEP), 0x01);
    ret();
}
/*
 * $80:86E2 Mode0E_TitleWait (M=1, X=1, RTS). At minute 4 the race segment
 * opens: switch to mode $0C (practice) and start fading out ($1923).
 */
void Mode0E_TitleWait_m1x1(void)
{
    lda8(rd8(LONG(BCAST_MINUTE)));
    cmp8((u8)A, 0x04);
    if (fc) {
        lda8(0x0C);
        wr8(DP(GAME_MODE), 0x0C);
        wr8(DP(MODE_ACTIVE), 0);
        wr8(DP(MODE_STEP), 0);
        wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
    }
    ret();
}
/* $80:89C6 FinishModeSetup (RTS): last step of a setup list. Switch to the
 * mode's main handler ($75 = 1, step 0), set $79, start a fade-in. */
void FinishModeSetup_m1x1(void)
{
    lda8(0x01);
    wr8(DP(0x79), 0x01);
    wr8(DP(MODE_ACTIVE), 0x01);
    SET_A8(lsr8(0x01));                     /* A = 0, C = 1 */
    wr8(DP(MODE_STEP), 0x00);
    lda8(0x01);
    wr8(ABS(FADE_STATE), 0x01);                 /* fade in */
    ret();
}
/*
 * $80:D177 StepObjectX (RTL). Object X ($1500 table, 16-bit position
 * $1500/$1501 with fraction $1520) moves by its velocity $1510: the high
 * nibble is the signed whole part, the low nibble the fraction (in 16ths).
 * Leaves A = carry out of the fraction + whole part.
 */
void StepObjectX_m1x1(void)
{
    u8 v = rd8(ABS(MARKER_VX + X));
    wr8(DP(0x01), (u8)(v << 4));               /* fraction */
    u8 whole = lsr8(lsr8(lsr8(lsr8(v))));
    cmp8(whole, 0x08);
    if (whole >= 0x08)
        whole |= 0xF0;                         /* sign-extend the nibble */
    wr8(DP(0x00), whole);
    ldy8(whole & 0x80 ? 0xFF : 0x00);
    wr8(DP(0x02), (u8)Y);                      /* high byte of the step */
    lda8(rd8(ABS(MARKER_FRAC + X)));
    fc = 0;
    adc8(rd8(DP(0x01)));
    wr8(ABS(MARKER_FRAC + X), (u8)A);
    u8 carry = fc;
    push8(carry);                              /* PHA */
    lda8(rd8(ABS(MARKER_X + X)));
    fc = carry;
    adc8(rd8(DP(0x00)));
    wr8(ABS(MARKER_X + X), (u8)A);
    lda8(rd8(ABS(MARKER_X + 0x01 + X)));
    adc8(rd8(DP(0x02)));
    wr8(ABS(MARKER_X + 0x01 + X), (u8)A);
    lda8(pull8());
    fc = 0;
    adc8(rd8(DP(0x00)));
    retl();
}
/*
 * $80:CF66 ComputePlaces (RTS). For every rider X: place = 8 minus the
 * riders it is ahead of: riders with lower progress, and riders with equal
 * progress and a lower index.
 */
void ComputePlaces_m1x1(void)
{
    rep(0x20);
    for (int x = 0x0E; x >= 0; x -= 2) {
        X = (u16)x;
        wr8(DP(0x00), (u8)x);
        A = 0x0008;
        wr16(ABS(PLAYER_PLACE + x), 0x0008);
        for (int y = 0x0E; y >= 0; y -= 2) {
            Y = (u16)y;
            if ((u8)y == rd8(DP(0x00)))
                continue;
            u16 mine = rd16(ABS(RIDER_DIST + x)), theirs = rd16(ABS(RIDER_DIST + y));
            A = mine;
            if (mine < theirs)
                continue;
            if (mine == theirs && (u8)y >= rd8(DP(0x00)))
                continue;
            wr16(ABS(PLAYER_PLACE + x), dec16(rd16(ABS(PLAYER_PLACE + x))));
        }
    }
    X = 0xFE;
    Y = 0xFE;
    sep(0x20);
    ret();
}
/* $81:A025 ApplyVelocity (RTL): Y position $0289/$028A,X += speed $02A9,X
 * (fraction $02C9,X). */
void ApplyVelocity_m1x1(void)
{
    apply_speed(0x02AF, 0x02CF, 0x028F);
    retl();
}
/* $80:D231 ClearWords1530 (RTL): zero the low bytes of the four words at
 * $1530-$1537 (X is preserved on the stack). */
void ClearWords1530_m1x1(void)
{
    push8((u8)X);
    for (int x = 6; x >= 0; x -= 2)
        wr8(ABS(MARKER_ON + x), 0);
    X = pull8();
    nz8((u8)X);
    retl();
}
/* $81:F999 ResetMinuteFrame (RTS): restart the frame-within-minute counter
 * $7F:FFFA (16-bit) and return X = 1; A is preserved. */
void ResetMinuteFrame_m1x1(void)
{
    push8((u8)A);
    rep(0x20);
    lda16(0x0000);
    wr16(LONG(BCAST_FRAME), 0x0000);
    sep(0x20);
    ldx8(0x01);
    lda8(pull8());
    ret();
}
/* $80:BA6F Nop_BD39 (RTS): eight NOPs, an empty hook. */
void Nop_BD39_m1x1(void)
{
    ret();
}
/* $80:C0A8 Tick08CC (RTS): count down the per-rider timer $08CC,X; it is
 * cancelled (zeroed) while any of bits 6-7 of $0178,X are set. */
void Tick08CC_m1x1(void)
{
    lda8(rd8(ABS(RIDER_COUNTDOWN + X)));
    if (fz) { ret(); return; }
    wr8(ABS(RIDER_COUNTDOWN + X), dec8(rd8(ABS(RIDER_COUNTDOWN + X))));
    lda8(rd8(ABS(RIDER_INPUT + X)));
    and8(0xC0);
    if (fz) { ret(); return; }
    lda8(0x00);
    wr8(ABS(RIDER_COUNTDOWN + X), 0x00);
    ret();
}
/* $80:C74A DivideAX (RTS): divide A by X by repeated subtraction while the
 * result stays positive (signed): X = quotient, A = remainder. */
void DivideAX_m1x1(void)
{
    u8 d = (u8)X;
    wr8(DP(0x0F), d);
    ldx8(0x00);
    do {
        WAIT_TICK();
        X = inc8((u8)X);
        fc = 1;
        sbc8(rd8(DP(0x0F)));
    } while (!fn);
    X = dec8((u8)X);
    adc8(rd8(DP(0x0F)));
    ret();
}
/* $80:DBCF BlankQueueRow (RTS): 15 blank tiles ($00FC) at $7F:000E. */
void BlankQueueRow_m1x1(void)
{
    fill_queue_row(0x1E, 0x00FC);
    ret();
}
/* $81:8C2A BlankQueueRowPal2 (RTL): 10 blank tiles, palette 2 ($08FC). */
void BlankQueueRowPal2_m1x1(void)
{
    fill_queue_row(0x14, 0x08FC);
    retl();
}
/* $80:9DAF BeginFadeIn (RTS): start fading in ($1923 + 1), advance the mode
 * flag $75, set the mode timer $88 to 900 frames, main screen BG1+OBJ
 * ($1905 = $11) and clear $190D. */
void BeginFadeIn_m1x1(void)
{
    wr8(ABS(FADE_STATE), inc8(rd8(ABS(FADE_STATE))));
    wr8(DP(MODE_ACTIVE), inc8(rd8(DP(MODE_ACTIVE))));
    rep(0x20);
    lda16(0x0384);
    wr16(DP(MODE_TIMER), 0x0384);
    sep(0x20);
    lda8(0x11);
    wr8(ABS(SH_TM), 0x11);
    wr8(ABS(SH_CGADSUB), 0);
    ret();
}
/* $80:AB3D SendSpeedToApu (RTS): engine pitch = player speed ($01C0:$01B8)
 * >> 3, low byte to APUIO2. */
void SendSpeedToApu_m1x1(void)
{
    lda8(rd8(ABS(RIDER_SPEED_HI)));
    A = (u16)((A << 8) | (A >> 8));             /* XBA */
    nz8((u8)A);
    lda8(rd8(ABS(RIDER_SPEED_LO)));
    rep(0x20);
    lda16(lsr16(lsr16(lsr16(A))));
    sep(0x20);
    wr8(ABS(APUIO2), (u8)A);
    ret();
}
/*
 * $80:BCB2 RiderShadowY (RTS): $0979,X = ($090C,X - $0190,X - 8) * 2
 * + $01F0,X + 4, with the original's carry chaining (the second SBC and ADC
 * use the carry of the step before).
 */
void RiderShadowY_m1x1(void)
{
    lda8(rd8(ABS(RIDER_RUNOUT + X)));
    fc = 1;
    sbc8(rd8(ABS(RIDER_X + X)));
    sbc8(0x08);
    lda8(asl8((u8)A));
    fc = 0;
    adc8(rd8(ABS(RIDER_LANE + X)));
    adc8(0x04);
    wr8(ABS(0x0979 + X), (u8)A);
    ret();
}
/*
 * $80:A11C Add32 (RTS): the 32-bit value at $10-$13 += $04-$07. The high
 * word adds the low word's carry first ($0000 + $12 + C), then $06.
 */
void Add32_m1x1(void)
{
    rep(0x20);
    lda16(rd16(DP(0x04)));
    fc = 0;
    adc16(rd16(DP(0x10)));
    wr16(DP(0x10), A);
    lda16(0x0000);
    adc16(rd16(DP(0x12)));
    fc = 0;
    adc16(rd16(DP(0x06)));
    wr16(DP(0x12), A);
    sep(0x20);
    ret();
}
/* $80:D971 DispatchBy85 (RTS): tail-jump through the routine table at
 * $80:D409 indexed by $85 (pointer kept at $00/$01, X = 0). */
void DispatchBy85_m1x1(void)
{
    lda8(rd8(DP(SUB_STEP)));
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABS(0xD23D + Y))); wr8(DP(0x00), (u8)A);
    Y = inc8((u8)Y);
    lda8(rd8(ABS(0xD23D + Y))); wr8(DP(0x01), (u8)A);
    ldx8(0x00);
    r816_call_pc(0x800000u | rd16(LONG(0x000000)));
}
/* $80:F394 StartMode01 (RTS): mode 1 from step 0 with a 1800-frame timer
 * ($88), start fading out ($1923 - 1), return C = 1. */
void StartMode01_m1x1(void)
{
    rep(0x20);
    lda16(0x0708);
    wr16(DP(MODE_TIMER), 0x0708);
    sep(0x20);
    lda8(0x01);
    wr8(DP(GAME_MODE), 0x01);
    wr8(DP(MODE_ACTIVE), 0);
    wr8(DP(MODE_STEP), 0);
    wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
    fc = 1;
    ret();
}
/* $80:9896 BsxReadSerialByte (RTL): clock 8 bits out of the BS-X serial
 * port $2199 (write $80 then $81, read bit 7), MSB first, into A. */
void BsxReadSerialByte_m1x1(void)
{
    ldy8(0x08);
    do {
        lda8(0x80); wr8(ABS(0x2199), 0x80);
        lda8(0x81); wr8(ABS(0x2199), 0x81);
        lda8(rd8(ABS(0x2199)));
        lda8(asl8((u8)A));                     /* bit into carry */
        A = (u16)((A << 8) | (A >> 8)); nz8((u8)A);
        lda8(rol8((u8)A));                     /* shift it into the byte */
        A = (u16)((A << 8) | (A >> 8)); nz8((u8)A);
        Y = dec8((u8)Y);
    } while (!fz);
    A = (u16)((A << 8) | (A >> 8)); nz8((u8)A);
    retl();
}
/* $80:9F7F GrowBar087E (RTS): while $087E < 15, count it up and redraw a
 * symmetric bar: $0474 = old length, and the length-1 ... 1 values going
 * out both ways from $0455+$1D/$0455+$21. */
void GrowBar087E_m1x1(void)
{
    lda8(rd8(ABS(BAR_LENGTH)));
    cmp8((u8)A, 0x0F);
    if (fz) { ret(); return; }
    wr8(ABS(BAR_LENGTH), inc8(rd8(ABS(BAR_LENGTH))));
    wr8(ABS(0x0471), (u8)A);
    ldx8(0x1D);
    ldy8(0x21);
    for (;;) {
        lda8(dec8((u8)A));
        if (fz) break;
        wr8(ABS(0x0452 + X), (u8)A);
        wr8(ABS(0x0452 + Y), (u8)A);
        Y = inc8((u8)Y); Y = inc8((u8)Y);
        X = dec8((u8)X); X = dec8((u8)X);
        if (fn) break;
    }
    ret();
}
/* $80:9FBF StepColourCycle (RTS): every 14 frames ($0882 countdown), copy
 * the next colour word of the list at $88:BC00 (index $0880, $FF = stopped)
 * to $1C00 and flag the palette upload ($1915). */
void StepColourCycle_m1x1(void)
{
    lda8(rd8(ABS(COLOUR_CYCLE)));
    lda8(inc8((u8)A));
    if (!fz) {
        lda8(rd8(ABS(COLOUR_TIMER)));
        if (fz) {
            X = rd8(ABS(COLOUR_CYCLE));
            rep(0x20);
            lda16(rd16(LONGX(0x9CD000)));
            wr16(ABS(PALETTE_BUF), A);
            sep(0x20);
            wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
            wr8(ABS(COLOUR_CYCLE), inc8(rd8(ABS(COLOUR_CYCLE))));
            wr8(ABS(COLOUR_CYCLE), inc8(rd8(ABS(COLOUR_CYCLE))));
            lda8(0x0E);
            wr8(ABS(COLOUR_TIMER), 0x0E);
        }
    }
    wr8(ABS(COLOUR_TIMER), dec8(rd8(ABS(COLOUR_TIMER))));
    ret();
}
/* $80:A374 MergeBsxFlags (RTL): OR the BS-X player data bits into the
 * unlock flags: $0856 |= $11:5719|$11:571D, $0857 |= $571A|$571E,
 * $0858 |= $571B|$571F, $0859 |= ($5716 << 4) | $571C. */
void MergeBsxFlags_m1x1(void)
{
    static const u16 dst[3] = { 0x0856, 0x0857, 0x0858 };
    for (int i = 0; i < 3; i++) {
        lda8(rd8(LONG(0x115719 + i)));
        ora8(rd8(LONG(0x11571D + i)));
        wr8(ABS(dst[i]), tsb8(rd8(ABS(dst[i]))));
    }
    lda8(rd8(LONG(SAVE_STADIUM)));
    for (int i = 0; i < 4; i++)
        lda8(asl8((u8)A));
    ora8(rd8(LONG(0x11571C)));
    wr8(ABS(ENTRY_CODE + 0x09), tsb8(rd8(ABS(ENTRY_CODE + 0x09))));
    retl();
}
/*
 * $80:B181 LaneFromY (RTS): for each of the 8 riders, $08A4,X = lane 5..1
 * from the track Y $01F0,X: 5 above Y $10, then one lane lower per 8 pixels.
 */
void LaneFromY_m1x1(void)
{
    ldx8(0x07);
    do {
        ldy8(0x05);
        lda8(rd8(ABS(RIDER_LANE + X)));
        fc = 1;
        sbc8(0x10);
        if (!fn) {
            for (;;) {
                Y = dec8((u8)Y);
                if (fz) break;
                fc = 1;
                sbc8(0x08);
                if (fn) break;
            }
        }
        lda8((u8)Y);
        wr8(ABS(RIDER_LANE_NUM + X), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    ret();
}
/*
 * $80:F8D9 MarkFastRider (RTS): a rider on the ground ($01E8,X = 0) with
 * $01E0,X < 7 (signed compare by N) going faster than $02:80 (speed
 * $01C0:$01B8) gets $01C8,X = $FF.
 */
void MarkFastRider_m1x1(void)
{
    lda8(rd8(ABS(RIDER_AIR + X)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(RIDER_LEAN + X)));
    cmp8((u8)A, 0x07);
    if (!fn) { ret(); return; }
    lda8(rd8(ABS(RIDER_SPEED_HI + X)));
    cmp8((u8)A, 0x03);
    if (!fc) {
        cmp8((u8)A, 0x02);
        if (!fz) { ret(); return; }
        lda8(rd8(ABS(RIDER_SPEED_LO + X)));
        lda8(asl8((u8)A));
        if (!fc) { ret(); return; }
    }
    lda8(0xFF);
    wr8(ABS(RIDER_BUMPED + X), 0xFF);
    ret();
}
/*
 * $80:CD17 DrawRiderShadowPair (RTS): two 8x8 sprites in the OAM buffer at
 * $16C0,Y and $16C4,Y: Y = A - 7 and A - 15, X = $01B0,X + 16 and + 8,
 * tiles $E0 and $F0.
 */
void DrawRiderShadowPair_m1x1(void)
{
    fc = 0; adc8(0xF9); wr8(ABS(OAM_BUF + 0xC0 + Y), (u8)A);
    fc = 0; adc8(0xF8); wr8(ABS(OAM_BUF + 0xC4 + Y), (u8)A);
    lda8(rd8(ABS(RIDER_Y + X)));
    fc = 0; adc8(0x08); wr8(ABS(OAM_BUF + 0xC5 + Y), (u8)A);
    fc = 0; adc8(0x08); wr8(ABS(OAM_BUF + 0xC1 + Y), (u8)A);
    lda8(0xE0); wr8(ABS(OAM_BUF + 0xC2 + Y), 0xE0);
    lda8(0xF0); wr8(ABS(OAM_BUF + 0xC6 + Y), 0xF0);
    ret();
}
/*
 * $80:BD2B ScrollRivalsRelative (RTS): for riders 7..1 that are on the course
 * ($01D8,X != 0), move them by their own speed $0180,X minus the player's
 * $0180 (the camera follows the player): position $0198:$0190,X and the
 * column-relative $090C,X.
 */
void ScrollRivalsRelative_m1x1(void)
{
    ldx8(0x07);
    do {
        lda8(rd8(ABS(RIDER_ACTIVE + X)));
        if (!fz) {
            lda8(rd8(ABS(RIDER_STEP + X)));
            fc = 0;
            adc8(rd8(ABS(RIDER_X + X)));
            if (fc)
                wr8(ABS(RIDER_PAGE + X), inc8(rd8(ABS(RIDER_PAGE + X))));
            fc = 1;
            sbc8(rd8(ABS(RIDER_STEP)));
            wr8(ABS(RIDER_X + X), (u8)A);
            if (!fc)
                wr8(ABS(RIDER_PAGE + X), dec8(rd8(ABS(RIDER_PAGE + X))));
            lda8(rd8(ABS(RIDER_RUNOUT + X)));
            fc = 1;
            sbc8(rd8(ABS(RIDER_STEP)));
            wr8(ABS(RIDER_RUNOUT + X), (u8)A);
        }
        X = dec8((u8)X);
    } while (!fz);
    ret();
}
/*
 * $80:C758 RiderLaneBand (RTS): $0958,X = which band of the track the rider
 * is in by Y position $01F0,X against the 5 limits at $80:BE1D (signed
 * compares), 0 while airborne ($01D0,X != 0) or past the last limit; bands
 * are 2 higher when $0208,X = 1.
 */
void RiderLaneBand_m1x1(void)
{
    ldy8(0x00);
    lda8(rd8(ABS(RIDER_TIMER + X)));
    if (fz) {
        lda8(rd8(ABS(RIDER_LANE + X)));
        for (;;) {
            cmp8((u8)A, rd8(ABS(0xBB3E + Y)));
            if (fn) break;
            Y = inc8((u8)Y);
            cmp8((u8)Y, 0x05);
            if (!fn) { ldy8(0x00); break; }
        }
        lda8((u8)Y);
        if (!fz) {
            lda8(0x01);
            cmp8(0x01, rd8(ABS(RIDER_JUMP + X)));
            if (fz) {
                Y = inc8((u8)Y);
                Y = inc8((u8)Y);
            }
        }
    }
    lda8((u8)Y);
    wr8(ABS(RIDER_BAND + X), (u8)A);
    ret();
}
/*
 * $80:D606 SetupSpriteRow15 (RTS): fill 15 OAM entries at $1700 from the
 * tables at $80:D7F5 (X), $80:D804 (Y) and $80:D7E6 (tile + $A0), attribute
 * $31, size bits $1860,X = 2 (large); $1958 counts up.
 */
void SetupSpriteRow15_m1x1(void)
{
    wr8(ABS(SPRITES_FROZEN), inc8(rd8(ABS(SPRITES_FROZEN))));
    ldx8(0x0E);
    do {
        lda8((u8)X);
        lda8(asl8((u8)A));
        lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABS(0xD641 + X))); wr8(ABS(OAM_BUF + 0x100 + Y), (u8)A);
        lda8(rd8(ABS(0xD650 + X))); wr8(ABS(OAM_BUF + 0x101 + Y), (u8)A);
        lda8(rd8(ABS(0xD632 + X)));
        fc = 1;
        sbc8(0x60);
        wr8(ABS(OAM_BUF + 0x102 + Y), (u8)A);
        lda8(0x31); wr8(ABS(OAM_BUF + 0x103 + Y), 0x31);
        lda8(0x02); wr8(ABS(OAM_SIZE_BUF + 0x40 + X), 0x02);
        X = dec8((u8)X);
    } while (!fn);
    ret();
}
/*
 * $80:C17C RiderHeightStep (RTS), with the caller's carry: 16-bit
 * $08EC:$08E4,X += $08FC,X; then A = $08EC,X + $01 (+ carry), and $01 =
 * A - $08DC,X. On a carry out of the first add, $0904,X is halved; on a
 * borrow in the subtract, $0904,X = 1. Returns A = $01 with the subtract's
 * carry.
 */
void RiderHeightStep_m1x1(void)
{
    lda8(rd8(ABS(RIDER_HEIGHT_LO + X)));
    adc8(rd8(ABS(RIDER_FALL_SPEED + X)));
    wr8(ABS(RIDER_HEIGHT_LO + X), (u8)A);
    lda8(rd8(ABS(RIDER_HEIGHT_HI + X)));
    adc8(0x00);
    wr8(ABS(RIDER_HEIGHT_HI + X), (u8)A);
    adc8(rd8(DP(0x01)));
    if (fc)
        wr8(ABS(0x0904 + X), lsr8(rd8(ABS(0x0904 + X))));
    fc = 1;
    sbc8(rd8(ABS(RIDER_BOUNCE_HI + X)));
    wr8(DP(0x01), (u8)A);
    if (!fc) {
        lda8(0x01);
        wr8(ABS(0x0904 + X), 0x01);
    }
    lda8(rd8(DP(0x01)));
    ret();
}
/*
 * $81:9FCE ObjectFacing (RTL): $02D9,X = the direction of the larger of the
 * two speed components $0299,X and $02A9,X (signed): 3/2 for $0299 positive
 * / negative, 1/0 for $02A9; unchanged when they are equal. Y is preserved.
 */
void ObjectFacing_m1x1(void)
{
    push8((u8)Y);
    ldy8(0x03);
    lda8(rd8(ABS(OBJ_VX + X)));
    if (fn) {
        Y = dec8((u8)Y);
        eor8(0xFF);
        lda8(inc8((u8)A));
    }
    wr8(DP(0x00), (u8)A);
    wr8(DP(0x02), (u8)Y);
    ldy8(0x01);
    lda8(rd8(ABS(OBJ_VY + X)));
    if (fn) {
        Y = dec8((u8)Y);
        eor8(0xFF);
        lda8(inc8((u8)A));
    }
    cmp8((u8)A, rd8(DP(0x00)));
    if (!fz) {
        if (!fc)
            ldy8(rd8(DP(0x02)));
        lda8((u8)Y);
        wr8(ABS(0x02DF + X), (u8)A);
    }
    Y = pull8();
    nz8((u8)Y);
    retl();
}
/*
 * $80:FE35 AddScore (RTS): add the six decimal digits $09AC-$09B1 to the
 * score digits $0CE5-$0CEA (index 5 = units), carrying tens up, then clear
 * the addend. (A digit that does not carry leaves its "minus 10" in $0CE4.)
 */
void AddScore_m1x1(void)
{
    ldx8(0x05);
    fc = 0;
    for (;;) {
        lda8(rd8(ABS(SCORE_DIGITS + X)));
        adc8(rd8(ABS(SCORE_ADD + X)));
        wr8(ABS(SCORE_DIGITS + X), (u8)A);
        X = dec8((u8)X);
        if (fn) break;
        fc = 1;
        sbc8(0x0A);
        if (fc) {
            wr8(ABS(SCORE_DIGITS + 0x01 + X), (u8)A);
            fc = 1;
        } else {
            wr8(ABS(0x0CE4), (u8)A);
            fc = 0;
        }
    }
    lda8(0x00);
    ldx8(0x05);
    do {
        wr8(ABS(SCORE_ADD + X), 0x00);
        X = dec8((u8)X);
    } while (!fn);
    ret();
}
/* $81:9FF7 ApplyVelocityX (RTL): X position $0279/$027A,X += speed $0299,X
 * (fraction $02B9,X). */
void ApplyVelocityX_m1x1(void)
{
    apply_speed(0x029F, 0x02BF, 0x027F);
    retl();
}
/*
 * $81:93FE SpawnPlayerSpark (RTL): start effect slot 0 at the player's
 * position ($0190, $01B0): kind 3 ($0110), 3 frames ($0120), speeds
 * $F8/$D0 ($0138/$0140), and set $09AD = 1.
 */
void SpawnPlayerSpark_m1x1(void)
{
    push8(DB);
    lda8(0x81);
    push8(0x81);
    DB = pull8(); nz8(DB);
    push8((u8)X);
    ldx8(0x00);
    lda8(rd8(ABS(RIDER_X))); wr8(ABS(EFFECT_X + X), (u8)A);
    lda8(rd8(ABS(RIDER_Y))); wr8(ABS(EFFECT_Y + X), (u8)A);
    lda8(0x00); wr8(ABS(EFFECT_FRAME + X), 0x00);
    lda8(0x03); wr8(ABS(EFFECT_KIND + X), 0x03);
    lda8(0xF8); wr8(ABS(EFFECT_VX + X), 0xF8);
    lda8(0xD0); wr8(ABS(EFFECT_VY + X), 0xD0);
    lda8(0x03); wr8(ABS(EFFECT_TIMER + X), 0x03);
    wr8(ABS(EFFECT_STEP + X), 0);
    lda8(0x01); wr8(ABS(SCORE_ADD + 0x01), 0x01);
    X = pull8(); nz8((u8)X);
    DB = pull8(); nz8(DB);
    retl();
}
/*
 * $81:96C2 StepSpark (RTS), effect slot Y: count its frames $0120 down (kind
 * $0110 cleared at zero), rise one pixel every other frame, and draw it as
 * sprite (Y + 4) in the OAM buffer: tile $8E, attribute $2D, large.
 */
void StepSpark_m1x1(void)
{
    X = Y; nz8((u8)X);
    u8 t = dec8(rd8(ABS(EFFECT_TIMER + X)));
    wr8(ABS(EFFECT_TIMER + X), t);
    if (t == 0)
        wr8(ABS(EFFECT_KIND + X), 0);
    lda8(rd8(ABS(EFFECT_TIMER + X)));
    lda8(lsr8((u8)A));
    if (fc)
        wr8(ABS(EFFECT_Y + X), dec8(rd8(ABS(EFFECT_Y + X))));
    lda8((u8)X);
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABS(EFFECT_X + X))); wr8(ABS(OAM_BUF + 0x10 + Y), (u8)A);
    lda8(rd8(ABS(EFFECT_Y + X))); wr8(ABS(OAM_BUF + 0x11 + Y), (u8)A);
    lda8(0x8E); wr8(ABS(OAM_BUF + 0x12 + Y), 0x8E);
    lda8(0x2D); wr8(ABS(OAM_BUF + 0x13 + Y), 0x2D);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02);
    wr8(ABS(OAM_SIZE_BUF + 0x04 + Y), 0x02);
    ret();
}
/* $80:DE46 QueueNameplate (RTS): queue the tile list at $80:DE4F at VRAM
 * $18F4 (JMP VramQueueTiles20). */
void QueueNameplate_m1x1(void)
{
    rep(0x20);
    lda16(0x18F4); wr16(DP(0x00), 0x18F4);
    lda16(0xDE58); wr16(DP(0x02), 0xDE58);
    sep(0x20);
    VramQueueTiles20_m1x1();
}
/* $80:A659 QueueResultsRow (RTS): unless $84 is set, queue the list at
 * $80:A666 at VRAM $1892 through $80:B38B. */
void QueueResultsRow_m1x1(void)
{
    lda8(rd8(DP(RACE_END)));
    if (!fz) { ret(); return; }
    rep(0x20);
    lda16(0x1892); wr16(DP(0x00), 0x1892);
    lda16(0xA66F); wr16(DP(0x02), 0xA66F);
    sep(0x20);
    r816_call_pc(0x80B394);
}
/* $81:C64E FadeApuVolume (RTS): step the sound volume $198A down by 4
 * (while non-zero) and send it to APUIO2. */
void FadeApuVolume_m1x1(void)
{
    lda8(rd8(ABS(ENGINE_PITCH)));
    if (!fz) {
        fc = 1;
        sbc8(0x04);
        wr8(ABS(ENGINE_PITCH), (u8)A);
    }
    lda8(rd8(ABS(ENGINE_PITCH)));
    wr8(ABS(APUIO2), (u8)A);
    ret();
}
/* $82:8CC5 ReadStreamByte (M=1, X=0, RTS): A = [$08]; advance the long
 * pointer $08-$0A, wrapping from $FFFF to $8000 of the next bank. */
void ReadStreamByte_m1x0(void)
{
    u32 p = rd16(DP(0x08)) | ((u32)rd8(DP(0x0A)) << 16);
    lda8(rd8(p & 0xFFFFFF));
    X = rd16(DP(0x08)); nz16(X);
    X = inc16(X);
    if (fz) {
        ldx16(0x8000);
        wr8(DP(0x0A), inc8(rd8(DP(0x0A))));
    }
    wr16(DP(0x08), X);
    ret();
}
/* $80:BCFC AddSpeedToX (RTS): rider X's position high $0186 = speed high
 * $01C6, then fraction $0914 += speed low $01BE with the carry into $0186. */
void AddSpeedToX_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    wr8(ABSX(RIDER_STEP), (u8)A);
    lda8(rd8(ABSX(RIDER_SPEED_LO)));
    fc = 0;
    adc8(rd8(ABSX(RIDER_X_FRAC)));
    wr8(ABSX(RIDER_X_FRAC), (u8)A);
    if (fc)
        wr8(ABSX(RIDER_STEP), inc8(rd8(ABSX(RIDER_STEP))));
    ret();
}
/* $80:EE5D BlankQueueRowB (RTS): fill words $7F:000E..$7F:002A with tile
 * $00FC. */
void BlankQueueRowB_m1x1(void)
{
    ldx8(0x1E);
    rep(0x20);
    lda16(0x00FC);
    do {
        wr16(LONGX(VQ_DATA), 0x00FC);
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fz);
    sep(0x20);
    ret();
}
/* $81:98A0 StepAnimTimer (RTS): for object $03A3, count the word timer
 * $031F down; at zero step the frame $030F and reload $00D0. */
void StepAnimTimer_m1x1(void)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    rep(0x20);
    u16 t = dec16(rd16(ABSX(OBJ_STEP)));
    wr16(ABSX(OBJ_STEP), t);
    if (t == 0) {
        wr16(ABSX(OBJ_STATE), inc16(rd16(ABSX(OBJ_STATE))));
        lda16(0x00D0);
        wr16(ABSX(OBJ_STEP), 0x00D0);
    }
    sep(0x20);
    ret();
}
/* $80:EBC5 DispatchBy85B (RTS): tail-jump through the table at $80:DFCF
 * indexed by $85 (pointer kept at $00/$01, X = 0). */
void DispatchBy85B_m1x1(void)
{
    lda8(rd8(DP(SUB_STEP)));
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xDFD8))); wr8(DP(0x00), (u8)A);
    Y = inc8((u8)Y);
    lda8(rd8(ABSY(0xDFD8))); wr8(DP(0x01), (u8)A);
    ldx8(0x00);
    r816_call_pc(0x800000u | rd16(LONG(0x000000)));
}
/* $80:BD16 ScrollByPlayerSpeed (RTS): scroll $9A += player speed $0186
 * (copied to $9C); on carry flip the nametable half $90 and count the page
 * $9D. */
void ScrollByPlayerSpeed_m1x1(void)
{
    lda8(rd8(DP(BG_SCROLL)));
    fc = 0;
    adc8(rd8(ABS(RIDER_STEP)));
    wr8(DP(BG_SCROLL), (u8)A);
    wr8(DP(SCROLL_X), (u8)A);
    if (fc) {
        lda8(rd8(DP(SCROLL_HALF)));
        eor8(0x01);
        wr8(DP(SCROLL_HALF), (u8)A);
        wr8(DP(SCROLL_PAGE), inc8(rd8(DP(SCROLL_PAGE))));
    }
    ret();
}
/* $80:DFC2 EndAtBonusMinute (RTS): at broadcast minute $38 or $1F switch to
 * mode $15 and leave the caller (drop its return, JMP $80:8F17). */
void EndAtBonusMinute_m1x1(void)
{
    lda8(rd8(LONG(BCAST_MINUTE)));
    cmp8((u8)A, 0x38);
    if (!fz) cmp8((u8)A, 0x1F);
    if (!fz) { ret(); return; }
    lda8(0x15);
    wr8(DP(GAME_MODE), 0x15);
    lda8(pull8());
    lda8(pull8());
    r816_call_pc(0x808F15);
}
/* $81:C5B5 ScrollJumpRider (RTS): $0196 += speed $0186 (carry into $019E),
 * $090C -= $0186. */
void ScrollJumpRider_m1x1(void)
{
    lda8(rd8(ABS(RIDER_STEP)));
    fc = 0;
    adc8(rd8(ABS(RIDER_X)));
    if (fc)
        wr8(ABS(RIDER_PAGE), inc8(rd8(ABS(RIDER_PAGE))));
    wr8(ABS(RIDER_X), (u8)A);
    lda8(rd8(ABS(RIDER_RUNOUT)));
    fc = 1;
    sbc8(rd8(ABS(RIDER_STEP)));
    wr8(ABS(RIDER_RUNOUT), (u8)A);
    ret();
}
/* $81:C608 DispatchByDistance (RTS): find the band of the word $1981 in the
 * thresholds at $81:C5FE (five words) and tail-jump through $81:C61F. */
void DispatchByDistance_m1x1(void)
{
    rep(0x20);
    ldx8(0x00);
    lda16(rd16(ABS(JUMP_RUN_FRAMES)));
    do {
        cmp16(A, rd16(ABSX(0xC5FE)));
        if (!fc) break;
        X = inc8((u8)X);
        X = inc8((u8)X);
        cmp8((u8)X, 0x0A);
    } while (!fz);
    sep(0x20);
    r816_call_pc(0x810000u | rd16(LONG(0x81C61F + X)));
}
/* $81:C743 SetJumpBanner (RTS): sprite $1610-$1613 = X $38, Y $199C, tile
 * $8E, attribute $2D; $1824 = 2. */
void SetJumpBanner_m1x1(void)
{
    lda8(0x38); wr8(ABS(OAM_BUF + 0x10), 0x38);
    lda8(rd8(ABS(0x199C))); wr8(ABS(OAM_BUF + 0x11), (u8)A);
    lda8(0x8E); wr8(ABS(OAM_BUF + 0x12), 0x8E);
    lda8(0x2D); wr8(ABS(OAM_BUF + 0x13), 0x2D);
    lda8(0x02); wr8(ABS(OAM_SIZE_BUF + 0x04), 0x02);
    ret();
}
/* $80:954C AddBcdDigits (RTS): add the digits $03D0-$03D4 into the decimal
 * number $154A-$154E (one digit per byte, carry into the byte before). */
void AddBcdDigits_m1x1(void)
{
    ldx8(0x04);
    do {
        lda8(rd8(ABSX(SCORE_COPY + 0x01)));
        fc = 0;
        adc8(rd8(ABSX(DIGIT_ROW)));
        cmp8((u8)A, 0x0A);
        if (fc) {
            sbc8(0x0A);
            wr8(ABSX(SCORE_COPY), inc8(rd8(ABSX(SCORE_COPY))));
        }
        wr8(ABSX(SCORE_COPY + 0x01), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    ret();
}
/* $81:BAC3, $81:BB23, $81:BD92 ObjectAdvance8/20/80 (RTS). */
void ObjectAdvance8_m1x1(void)  { object_advance(0x08, 0x06); ret(); }
void ObjectAdvance20_m1x1(void) { object_advance(0x20, 0x03); ret(); }
void ObjectAdvance80_m1x1(void) { object_advance(0x80, 0x02); ret(); }
/* $81:BED8, $81:BFB0 ObjectAdvance9/C0 (RTS). */
void ObjectAdvance9_m1x1(void)  { object_advance(0x09, 0x03); ret(); }
void ObjectAdvanceC0_m1x1(void) { object_advance(0xC0, 0x04); ret(); }
/* $81:BC60, $81:BD38 ObjectAdvanceOr4/Or5 (RTS): as above, but while the
 * counter runs $034F is 4 or 5. */
void ObjectAdvanceOr4_m1x1(void) { object_advance_or(0x15, 0x03, 0x04); ret(); }
void ObjectAdvanceOr5_m1x1(void) { object_advance_or(0x08, 0x06, 0x05); ret(); }
/* $82:86EB TickTimers (RTL): count the bytes $39+X down to zero, X = $16..0
 * each frame and $29..0 every eleventh frame (divider $38). */
void TickTimers_m1x1(void)
{
    ldx8(0x16);
    u8 d = dec8(rd8(DP(0x38)));
    wr8(DP(0x38), d);
    if (fn) {
        lda8(0x0A); wr8(DP(0x38), 0x0A);
        ldx8(0x29);
    }
    do {
        lda8(rd8(DPX(0x39)));
        if (!fz)
            wr8(DPX(0x39), dec8(rd8(DPX(0x39))));
        X = dec8((u8)X);
    } while (!fn);
    retl();
}
/* $80:9877 BsxReadHeader3 (RTL): with BS-X $2199 = $81 read three serial
 * bytes into $00-$02, then set $2199 back to $41. */
void BsxReadHeader3_m1x1(void)
{
    lda8(0x81); wr8(ABS(0x2199), 0x81);
    push8(0x80); push16(0x9881); BsxReadSerialByte_m1x1(); wr8(DP(0x00), (u8)A);
    push8(0x80); push16(0x9887); BsxReadSerialByte_m1x1(); wr8(DP(0x01), (u8)A);
    push8(0x80); push16(0x988D); BsxReadSerialByte_m1x1(); wr8(DP(0x02), (u8)A);
    lda8(0x41); wr8(ABS(0x2199), 0x41);
    retl();
}
/* $80:E756 CheckCourseTime (RTS): in the course's last broadcast minute
 * ($80:E76D by course $7C), once the frame count reaches $0708 set $CA. */
void CheckCourseTime_m1x1(void)
{
    X = rd8(DP(COURSE_SLOT)); nz8((u8)X);
    lda8(rd8(LONG(BCAST_MINUTE)));
    cmp8((u8)A, rd8(ABSX(0xE776)));
    if (fz) {
        rep(0x20);
        lda16(rd16(LONG(BCAST_FRAME)));
        cmp16(A, 0x0708);
        if (fc) {
            sep(0x20);
            lda8(0x01); wr8(ABS(0x00CA), 0x01);
        }
    }
    sep(0x20);
    ret();
}
/* $81:C7C3 ClearOnBonusTiles (RTL): for object X on track row $1F with tile
 * $3B-$3D under it, clear $020E,X and $08BC,X. */
void ClearOnBonusTiles_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_TILE)));
    cmp8((u8)A, 0x1F);
    if (fz) {
        lda8(rd8(ABSX(RIDER_TILE_Y)));
        cmp8((u8)A, 0x3B);
        if (!fz) cmp8((u8)A, 0x3C);
        if (!fz) cmp8((u8)A, 0x3D);
        if (fz) {
            wr8(ABSX(RIDER_JUMP), 0);
            wr8(ABSX(RIDER_ON_RAMP), 0);
        }
    }
    retl();
}
/* $81:C724 StepJumpBanner (RTL): while the banner is on ($199A) count its
 * timer $199B down (raising it, $199C - 1, while the timer is $20 or more)
 * and redraw it; at zero switch it off. */
void StepJumpBanner_m1x1(void)
{
    lda8(rd8(ABS(0x199A)));
    if (fz) { retl(); return; }
    lda8(rd8(ABS(0x199B)));
    if (fz) {
        wr8(ABS(0x199A), 0);
        retl();
        return;
    }
    cmp8((u8)A, 0x20);
    if (fc)
        wr8(ABS(0x199C), dec8(rd8(ABS(0x199C))));
    wr8(ABS(0x199B), dec8(rd8(ABS(0x199B))));
    push16(0xC741); SetJumpBanner_m1x1();
    retl();
}
/* $80:D8F8 CheckBonusTime (RTS): in broadcast minute $1E or $37, once the
 * frame count reaches $0708 set $CA. */
void CheckBonusTime_m1x1(void)
{
    lda8(rd8(LONG(BCAST_MINUTE)));
    cmp8((u8)A, 0x1E);
    if (!fz) cmp8((u8)A, 0x37);
    if (fz) {
        rep(0x20);
        lda16(rd16(LONG(BCAST_FRAME)));
        cmp16(A, 0x0708);
        if (fc) {
            sep(0x20);
            lda8(0x01); wr8(ABS(0x00CA), 0x01);
        }
    }
    sep(0x20);
    ret();
}
void Hundreds_m1x1(void) { hundreds(); }
void Hundreds_m0x1(void) { hundreds(); }
/* $80:9C66 SetupJumpScreen (RTS): $0450 = 8, $1905/$1906 = 2, BGMODE $25,
 * $79 = $75 = 1, $77 = 0, $A2 = 8. */
void SetupJumpScreen_m1x1(void)
{
    lda8(0x08); wr8(ABS(END_STEP), 0x08);
    lda8(0x02); wr8(ABS(SH_TM), 0x02); wr8(ABS(SH_TS), 0x02);
    lda8(0x25); wr8(ABS(BGMODE), 0x25);
    lda8(0x01); wr8(DP(0x79), 0x01); wr8(DP(MODE_ACTIVE), 0x01);
    lda8(lsr8((u8)A)); wr8(DP(MODE_STEP), (u8)A);
    lda8(0x08); wr8(ABS(ZOOM), 0x08);
    ret();
}
/* $80:BC4A RiderStartLane (RTS): for rider X with $56,X clear: rider 0
 * waits for $0940 (returns while it is set); others get $3D,X = X * 8 +
 * $1C. Then count $01D6,X up. */
void RiderStartLane_m1x1(void)
{
    lda8(rd8(DPX(0x56)));
    if (!fz) { ret(); return; }
    lda8((u8)X);
    if (fz) {
        lda8(rd8(ABS(OVERHEAT_LOCK)));
        if (!fz) { ret(); return; }
    } else {
        lda8((u8)X);
        lda8(asl8((u8)A));
        lda8(asl8((u8)A));
        lda8(asl8((u8)A));
        adc8(0x1C);
        wr8(DPX(RIDER_HOLD), (u8)A);
    }
    wr8(ABSX(RIDER_TIMER), inc8(rd8(ABSX(RIDER_TIMER))));
    ret();
}
void ScrollBySpeed16_m1x1(void)  { scroll_by_speed16(); }
void ScrollBySpeed16B_m1x1(void) { scroll_by_speed16(); }
/* $80:DF5E SetBgTileBase (RTL): $0086 = ($63 & $30) << 5 + $E000. */
void SetBgTileBase_m1x1(void)
{
    rep(0x20);
    lda16(rd16(DP(FRAME_COUNT)));
    and16(0x0030);
    lda16(asl16(A)); lda16(asl16(A)); lda16(asl16(A)); lda16(asl16(A)); lda16(asl16(A));
    fc = 0; adc16(0x2000);
    fc = 0; adc16(0xC000);
    wr16(ABS(UPLOAD_SRC), A);
    sep(0x20);
    retl();
}
/* $80:E4B2 SetupCourseScreen (RTS): $09C2 = $85 / 4; BG1SC = $1900 = 3,
 * BG2SC = $43, BG12NBA = $55, $01FE = $30, $1958 = 0. */
void SetupCourseScreen_m1x1(void)
{
    lda8(rd8(DP(SUB_STEP)));
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    wr8(ABS(0x09C2), (u8)A);
    lda8(0x03); wr8(ABS(BG1SC), 0x03); wr8(ABS(0x1900), 0x03);
    lda8(0x43); wr8(ABS(BG2SC), 0x43);
    lda8(0x55); wr8(ABS(BG12NBA), 0x55);
    lda8(0x30); wr8(ABS(RIDER_HEIGHT), 0x30);
    wr8(ABS(SPRITES_FROZEN), 0);
    ret();
}
/* $81:B929 StepObjectScript (RTS): for object $03A3, when its delay $036F
 * is zero step $031F; past 7 clear $02EF, else load $034F and the next
 * delay from the tables at $81:B94D and $81:B955. */
void StepObjectScript_m1x1(void)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(ABSX(OBJ_DELAY)));
    if (!fz) { ret(); return; }
    wr8(ABSX(OBJ_STEP), inc8(rd8(ABSX(OBJ_STEP))));
    lda8(rd8(ABSX(OBJ_STEP)));
    cmp8((u8)A, 0x08);
    if (fc) { wr8(ABSX(OBJ_ACTIVE), 0); ret(); return; }
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xB94D))); wr8(ABSX(OBJ_FRAME), (u8)A);
    lda8(rd8(ABSY(0xB955))); wr8(ABSX(OBJ_DELAY), (u8)A);
    ret();
}
void ReadStreamWord_m1x0(void)
{
    X = rd16(DP(0x08)); nz16(X);
    stream_step();
    A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
    stream_step();
    ret();
}
/* $81:BA63, $81:BB97, $81:BCD8, $81:BDF3, $81:BF4B, $81:BBF7
 * ObjectAdvanceRandom* (RTS). */
void ObjectAdvanceRandom20_m1x1(void) { object_advance_random(0x20, 0x01, 0x00, 0xBA73); ret(); }
void ObjectAdvanceRandom10_m1x1(void) { object_advance_random(0x10, 0x01, 0x00, 0xBBA7); ret(); }
void ObjectAdvanceRandom0A_m1x1(void) { object_advance_random(0x0A, 0x01, 0x00, 0xBCE8); ret(); }
void ObjectAdvanceRandom09_m1x1(void) { object_advance_random(0x09, 0x01, 0x00, 0xBE03); ret(); }
void ObjectAdvanceRandom10B_m1x1(void) { object_advance_random(0x10, 0x01, 0x00, 0xBF5B); ret(); }
void ObjectAdvanceRandom16_m1x1(void) { object_advance_random(0x16, 0x09, 0x09, 0xBC07); ret(); }
/* $81:BDAF ObjectCycleA, $81:BFCD ObjectCycleB (RTS). */
void ObjectCycleA_m1x1(void) { object_cycle(0x01, 0xBDD4, 0xBDD8, 0); ret(); }
void ObjectCycleB_m1x1(void) { object_cycle(0x02, 0xBFF4, 0xBFF8, 1); ret(); }
/* $80:D5E2 SetupBonusScreen (RTS): $09C2 = $85 / 4; BG1SC = $1900 = 3,
 * BG2SC = $43, BG12NBA = $55, BG3SC = $1A, BG34NBA = 1. */
void SetupBonusScreen_m1x1(void)
{
    lda8(rd8(DP(SUB_STEP)));
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    wr8(ABS(0x09C2), (u8)A);
    lda8(0x03); wr8(ABS(BG1SC), 0x03); wr8(ABS(0x1900), 0x03);
    lda8(0x43); wr8(ABS(BG2SC), 0x43);
    lda8(0x55); wr8(ABS(BG12NBA), 0x55);
    lda8(0x1A); wr8(ABS(BG3SC), 0x1A);
    lda8(0x01); wr8(ABS(BG34NBA), 0x01);
    ret();
}
/* $80:E6B4 FillBonusColumns (RTS): for X = 6..0 write the tiles $4C/$4B at
 * $09D3/$09D9 and $18 down rows $0A59, $0B59-$0C99 of column X. */
void FillBonusColumns_m1x1(void)
{
    static const u16 rows[] = { 0x0A59, 0x0B59, 0x0B99, 0x0BD9, 0x0C19, 0x0C59, 0x0C99 };
    ldx8(0x06);
    do {
        lda8(0x4C); wr8(ABSX(TRACK_ROWS), 0x4C);
        lda8(0x4B); wr8(ABSX(TRACK_ROWS + 0x06), 0x4B);
        lda8(0x18);
        for (int i = 0; i < 7; i++)
            wr8(ABSX(rows[i]), 0x18);
        X = dec8((u8)X);
    } while (!fn);
    ret();
}
/* $80:DF3A ResultsWait (RTS): leave on a new press ($21 | $26) or when the
 * word timer $88 runs out: clear $0CD7, $1E87 -= 10, mode 9 (JMP
 * $80:8F17). */
void ResultsWait_m1x1(void)
{
    lda8(rd8(DP(P1_PRESS_HI)));
    ora8(rd8(DP(P1_PRESS_LO)));
    if (fz) {
        rep(0x20);
        u16 t = dec16(rd16(DP(MODE_TIMER)));
        wr16(DP(MODE_TIMER), t);
        if (!fz) { sep(0x20); ret(); return; }
    }
    sep(0x20);
    wr8(ABS(ATTRACT_DEMO), 0);
    lda8(rd8(ABS(0x1E87)));
    fc = 1;
    sbc8(0x0A);
    wr8(ABS(0x1E87), (u8)A);
    lda8(0x09); wr8(DP(GAME_MODE), 0x09);
    r816_call_pc(0x808F15);
}
/* $80:E83E SetJumpRiderSprites (RTS): two sprites at OAM buffer $16C0,Y
 * for rider X: X positions A + $1F and + 8, Y positions from $01B6,X + 8
 * and + 16, tiles $E0 and $F0. */
void SetJumpRiderSprites_m1x1(void)
{
    fc = 0; adc8(0x1F); wr8(ABSY(OAM_BUF + 0xC0), (u8)A);
    fc = 0; adc8(0x08); wr8(ABSY(OAM_BUF + 0xC4), (u8)A);
    lda8(rd8(ABSX(RIDER_Y)));
    fc = 0; adc8(0x08); wr8(ABSY(OAM_BUF + 0xC5), (u8)A);
    fc = 0; adc8(0x08); wr8(ABSY(OAM_BUF + 0xC1), (u8)A);
    lda8(0xE0); wr8(ABSY(OAM_BUF + 0xC2), 0xE0);
    lda8(0xF0); wr8(ABSY(OAM_BUF + 0xC6), 0xF0);
    ret();
}
/* $80:ECC6 JumpRunUp (RTS): before the take-off ($84 = 0) scroll by the
 * speed (JMP ScrollByPlayerSpeed). After it, $0196 += speed; at $F0 or
 * more land ($84 = $019E = 2, sound $2D), else the speed is zero. */
void JumpRunUp_m1x1(void)
{
    lda8(rd8(DP(RACE_END)));
    if (fz) { ScrollByPlayerSpeed_m1x1(); return; }
    lda8(rd8(ABS(RIDER_STEP)));
    fc = 0;
    adc8(rd8(ABS(RIDER_X)));
    wr8(ABS(RIDER_X), (u8)A);
    cmp8((u8)A, 0xF0);
    if (fc) {
        lda8(0x02); wr8(DP(RACE_END), 0x02); wr8(ABS(RIDER_PAGE), 0x02);
        lda8(0x2D);
        push8(0x80); push16(0xECE7); QueueSoundB_m1x1();
        ret();
        return;
    }
    lda8(0x00); wr8(ABS(RIDER_STEP), 0x00);
    ret();
}
/* $82:84C5 ResetSoundState (RTL): $2C = $0F, $1903 = $80, $9A = 0,
 * $1959-$195C = 1-4, $1E8D-$1E90 = 0. */
void ResetSoundState_m1x1(void)
{
    lda8(0x0F); wr8(DP(RANDOM_LFSR), 0x0F);
    lda8(0x80); wr8(ABS(SH_INIDISP), 0x80);
    wr8(DP(BG_SCROLL), 0);
    lda8(0x01);
    for (u16 a = 0x1959; a <= 0x195C; a++) {
        wr8(ABS(a), (u8)A);
        if (a != 0x195C) lda8(inc8((u8)A));
    }
    for (u16 a = 0x1E8D; a <= 0x1E90; a++)
        wr8(ABS(a), 0);
    retl();
}
/* $80:B5CC StartBonusMode (RTS): sound 2, clear $1E92-$1E95, timer $88 =
 * $0708, $1991 = $0258, BG3SC $1A, BG34NBA 2, $1905 = 6, then
 * FinishModeSetup. */
void StartBonusMode_m1x1(void)
{
    lda8(0x02);
    push8(0x80); push16(0xB5D1); QueueSoundB_m1x1();
    for (u16 a = 0x1E92; a <= 0x1E95; a++)
        wr8(ABS(a), 0);
    rep(0x20);
    lda16(0x0708); wr16(DP(MODE_TIMER), 0x0708);
    lda16(0x0258); wr16(ABS(0x1991), 0x0258);
    sep(0x20);
    lda8(0x1A); wr8(ABS(BG3SC), 0x1A);
    lda8(0x02); wr8(ABS(BG34NBA), 0x02);
    lda8(0x06); wr8(ABS(SH_TM), 0x06);
    FinishModeSetup_m1x1();
}
/* $80:FAF2 StartWheelie (RTS): for the player (X = 0) with $0940, $60,
 * $01EE,X and $0CE0 all clear: $0990 = 8; the first time ($1979 clear)
 * $1979 = $30, sounds $30 and $2D, $AA = 0. Other riders with those flags
 * clear only get $0990 = 8. */
void StartWheelie_m1x1(void)
{
    lda8((u8)X);
    ora8(rd8(ABS(OVERHEAT_LOCK)));
    ora8(rd8(DP(OVERHEATED)));
    ora8(rd8(ABSX(RIDER_AIR)));
    ora8(rd8(ABS(TURBO_ON)));
    if (!fz) { ret(); return; }
    lda8(0x08); wr8(ABS(ENGINE_HEAT), 0x08);
    cmp8((u8)X, 0x00);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x1979)));
    if (!fz) { ret(); return; }
    lda8(0x30); wr8(ABS(0x1979), 0x30);
    push8(0x80); push16(0xFB16); QueueSoundB_m1x1();
    lda8(0x2D);
    push8(0x80); push16(0xFB1C); QueueSoundB_m1x1();
    wr8(DP(TURBO_WARN), 0);
    ret();
}
/* $80:862A StartIntroMode (RTS): from broadcast minute 3 on go straight to
 * mode 0 ($75 = $77 = 0); before it set BG3SC $2A, BG12NBA $44, BG34NBA 1,
 * timer $88 = $0A8C, step $1923 and FinishModeSetup. */
void StartIntroMode_m1x1(void)
{
    lda8(rd8(LONG(BCAST_MINUTE)));
    cmp8((u8)A, 0x03);
    if (fc) {
        lda8(0x00); wr8(ABS(GAME_MODE), 0x00);
        wr8(DP(MODE_ACTIVE), 0);
        wr8(DP(MODE_STEP), 0);
        ret();
        return;
    }
    lda8(0x2A); wr8(ABS(BG3SC), 0x2A);
    lda8(0x44); wr8(ABS(BG12NBA), 0x44);
    lda8(0x01); wr8(ABS(BG34NBA), 0x01);
    rep(0x20);
    lda16(0x0A8C); wr16(DP(MODE_TIMER), 0x0A8C);
    sep(0x20);
    wr8(ABS(FADE_STATE), inc8(rd8(ABS(FADE_STATE))));
    FinishModeSetup_m1x1();
}
/* $80:CF3B RankPlayer (RTS): while racing ($086E set, $84 clear, $92 set)
 * the place $086E = 8 minus the riders the word $085E beats among
 * $0860-$086C. */
void RankPlayer_m1x1(void)
{
    lda8(rd8(ABS(PLAYER_PLACE)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(RACE_END)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(RACE_STARTED)));
    if (fz) { ret(); return; }
    rep(0x20);
    lda16(0x0008); wr16(ABS(PLAYER_PLACE), 0x0008);
    ldy8(0x0E);
    lda16(rd16(ABS(RIDER_DIST)));
    do {
        cmp16(A, rd16(ABSY(RIDER_DIST)));
        if (fc && !fz)
            wr16(ABS(PLAYER_PLACE), dec16(rd16(ABS(PLAYER_PLACE))));
        Y = dec8((u8)Y);
        Y = dec8((u8)Y);
    } while (!fz);
    sep(0x20);
    ret();
}
/* $81:C178 ObjectWalkCycle (RTS): every 4 frames step $031F through 0-3
 * and count $027F; at $A0 reset the state ($030F = 0, delay $40). The frame
 * $034F comes from the table at $81:C1AA. */
void ObjectWalkCycle_m1x1(void)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(ABSX(OBJ_DELAY)));
    if (fz) {
        lda8(rd8(ABSX(OBJ_STEP)));
        lda8(inc8((u8)A));
        and8(0x03);
        wr8(ABSX(OBJ_STEP), (u8)A);
        lda8(0x04); wr8(ABSX(OBJ_DELAY), 0x04);
        wr8(ABSX(OBJ_X), inc8(rd8(ABSX(OBJ_X))));
        lda8(rd8(ABSX(OBJ_X)));
        cmp8((u8)A, 0xA0);
        if (fc) {
            wr8(ABSX(OBJ_STATE), 0);
            lda8(0x40); wr8(ABSX(OBJ_DELAY), 0x40);
        }
    }
    Y = rd8(ABSX(OBJ_STEP)); nz8((u8)Y);
    lda8(rd8(ABSY(0xC1AA)));
    wr8(ABSX(OBJ_FRAME), (u8)A);
    ret();
}
/* $82:8BB4 LoadStadiumPalette (RTL): copy 32 bytes from the Stadium's
 * table ($82:8BB0 indexed by $81:800C) to $1D20 and flag $1915. */
void LoadStadiumPalette_m1x1(void)
{
    push8(DB);
    lda8(0x82);
    push8(0x82);
    DB = pull8(); nz8(DB);
    rep(0x20);
    lda16(rd16(LONG(STADIUM_SELECT)));
    lda16(asl16(A));
    X = A; nz16(X);
    lda16(rd16(ABSX(0x8BB0)));
    wr16(DP(0x00), A);
    ldy8(0x1E);
    do {
        lda16(rd16(((u32)DB << 16) + ((rd16(DP(0x00)) + Y) & 0xFFFF)));
        wr16(ABSY(PALETTE_BUF + 0x120), A);
        Y = dec8((u8)Y);
        Y = dec8((u8)Y);
    } while (!fn);
    sep(0x20);
    wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:A3A5 CountCourseBits (RTL): $085A = the number of set bits in the
 * low six bits of $0850-$0859, modulo 64. */
void CountCourseBits_m1x1(void)
{
    wr8(DP(0x00), 0);
    ldx8(0x09);
    do {
        ldy8(0x05);
        lda8(rd8(ABSX(ENTRY_CODE)));
        wr8(DP(0x01), (u8)A);
        do {
            lda8(rd8(DP(0x01)));
            lda8(lsr8((u8)A));
            wr8(DP(0x01), (u8)A);
            lda8(rol8((u8)A));
            and8(0x01);
            fc = 0;
            adc8(rd8(DP(0x00)));
            wr8(DP(0x00), (u8)A);
            Y = dec8((u8)Y);
        } while (!fn);
        X = dec8((u8)X);
    } while (!fn);
    lda8(rd8(DP(0x00)));
    and8(0x3F);
    wr8(ABS(ENTRY_CODE + 0x0A), (u8)A);
    retl();
}
/* $80:B773 StartJumpMode (RTS): sound 2, clear the jump scores and flags
 * ($1578, $156B, $0CE0, words $1567, $1569, $156D), timer $88 = $0708,
 * $1905 = $13, BG3SC $1A, BG34NBA 2. */
void StartJumpMode_m1x1(void)
{
    lda8(0x02);
    push8(0x80); push16(0xB778); QueueSoundB_m1x1();
    wr8(ABS(0x1578), 0);
    wr8(ABS(JUMP_RANK), 0);
    wr8(ABS(TURBO_ON), 0);
    rep(0x20);
    wr16(ABS(JUMP_LAST_DIST), 0);
    wr16(ABS(JUMP_COUNT), 0);
    wr16(ABS(JUMP_BEST_SCORE), 0);
    lda16(0x0708); wr16(DP(MODE_TIMER), 0x0708);
    sep(0x20);
    lda8(0x15); wr8(ABS(SH_TM), 0x15);
    lda8(0x13); wr8(ABS(SH_TM), 0x13);
    lda8(0x1A); wr8(ABS(BG3SC), 0x1A);
    lda8(0x02); wr8(ABS(BG34NBA), 0x02);
    ret();
}
/* $80:DD70 ShowResultsBanner (RTS): $88 = $10, queue the lists at $80:DD98
 * (VRAM $18B6) and $80:DD9D (VRAM $0090, attribute 8), step $77, $10 = 0,
 * then JMP $80:DDAE. */
void ShowResultsBanner_m1x1(void)
{
    lda8(0x10); wr8(DP(MODE_TIMER), 0x10);
    rep(0x20);
    lda16(0x18B6); wr16(DP(0x00), 0x18B6);
    lda16(0xDDA1); wr16(DP(0x02), 0xDDA1);
    sep(0x20);
    push16(0xDD84); VramQueueTiles20_m1x1();
    rep(0x20);
    lda16(0x0090); wr16(DP(0x00), 0x0090);
    lda16(0xDDA6); wr16(DP(0x02), 0xDDA6);
    sep(0x20);
    lda8(0x08); wr8(DP(0x0F), 0x08);
    push16(0xDD99); VramQueueTiles_m1x1();
    wr8(DP(MODE_STEP), inc8(rd8(DP(MODE_STEP))));
    wr8(DP(0x10), 0);
    r816_call_pc(0x80DDB7);
}
/* $81:BA8A, $81:BBBE, $81:BCFF, $81:BE1A ObjectBlink*, $81:BF72 and
 * $81:BC20 ObjectRepeat4* (RTS). */
void ObjectBlinkA_m1x1(void) { object_repeat(1, 0, 0xBABF, 0xBAC1, 0xBAA8); ret(); }
void ObjectBlinkB_m1x1(void) { object_repeat(1, 0, 0xBBF3, 0xBBF5, 0xBBDC); ret(); }
void ObjectBlinkC_m1x1(void) { object_repeat(1, 0, 0xBD34, 0xBD36, 0xBD1D); ret(); }
void ObjectBlinkD_m1x1(void) { object_repeat(1, 0, 0xBE4F, 0xBE51, 0xBE38); ret(); }
void ObjectRepeat4A_m1x1(void) { object_repeat(3, 0, 0xBFA8, 0xBFAC, 0xBF91); ret(); }
void ObjectRepeat4B_m1x1(void) { object_repeat(3, 2, 0xBC58, 0xBC5C, 0xBC41); ret(); }
/* $81:BD57 ObjectRun4 (RTS): count the step 0-3; at 0 rest in state 2. */
void ObjectRun4_m1x1(void)
{
    if (object_ready()) {
        lda8(rd8(ABSX(OBJ_STEP)));
        lda8(inc8((u8)A));
        and8(0x03);
        wr8(ABSX(OBJ_STEP), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        if (fz) object_rest(2, 0xBD73);
        else object_frame(0xBD8A, 0xBD8E);
    }
    ret();
}
/* $81:BAE0 ObjectRun6 (RTS): count the step 0-5; at 0 rest in state 2. */
void ObjectRun6_m1x1(void)
{
    if (object_ready()) {
        lda8(rd8(ABSX(OBJ_STEP)));
        lda8(inc8((u8)A));
        cmp8((u8)A, 0x06);
        if (fc) lda8(0x00);
        wr8(ABSX(OBJ_STEP), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        if (fz) object_rest(2, 0xBB00);
        else object_frame(0xBB17, 0xBB1D);
    }
    ret();
}
/* $81:BEF5 ObjectRunOnce6 (RTS): step 1-5 through the tables; at 6 rest in
 * state 4 (the step stays at 6). */
void ObjectRunOnce6_m1x1(void)
{
    if (object_ready()) {
        lda8(rd8(ABSX(OBJ_STEP)));
        lda8(inc8((u8)A));
        wr8(ABSX(OBJ_STEP), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        cmp8((u8)A, 0x06);
        if (fc) object_rest(4, 0xBF11);
        else object_frame(0xBF28, 0xBF2E);
    }
    ret();
}
/* $81:BC7F ObjectLoop7 (RTS): step 1-6 through the tables; at 7 back to
 * step 0 with a random pause of $40-$7F frames as its delay. */
void ObjectLoop7_m1x1(void)
{
    if (object_ready()) {
        lda8(rd8(ABSX(OBJ_STEP)));
        lda8(inc8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        wr8(ABSX(OBJ_STEP), (u8)A);
        cmp8((u8)A, 0x07);
        if (fc) {
            ldy8(0x00);
            wr8(ABSX(OBJ_STEP), 0);
            random_rest(0xBC9B);
            wr8(ABSX(OBJ_DELAY), (u8)A);
        } else {
            lda8(rd8(ABSY(0xBCB3))); wr8(ABSX(OBJ_DELAY), (u8)A);
        }
        lda8(rd8(ABSY(0xBCBA))); wr8(ABSX(OBJ_FRAME), (u8)A);
    }
    ret();
}
/* $81:BB40 ObjectLoop5 (RTS): step 1-4 through the tables; at 5 back to
 * step 1 with the delay from the table plus a random 0-$3F. */
void ObjectLoop5_m1x1(void)
{
    if (object_ready()) {
        lda8(rd8(ABSX(OBJ_STEP)));
        lda8(inc8((u8)A));
        cmp8((u8)A, 0x05);
        if (fc) {
            lda8(0x01); wr8(ABSX(OBJ_STEP), 0x01);
            Y = 0x01; nz8(1);
            push8(0x81); push16(0xBB59); RandomB_m1x1();
            and8(0x3F);
            fc = 0;
            adc8(rd8(ABSY(0xBB76)));
            wr8(ABSX(OBJ_DELAY), (u8)A);
        } else {
            wr8(ABSX(OBJ_STEP), (u8)A);
            Y = (u8)A; nz8((u8)Y);
            lda8(rd8(ABSY(0xBB76))); wr8(ABSX(OBJ_DELAY), (u8)A);
        }
        lda8(rd8(ABSY(0xBB7B))); wr8(ABSX(OBJ_FRAME), (u8)A);
    }
    ret();
}
/* $81:BE53 ObjectFlicker (RTS): every 10 frames flip the frame bit; when
 * the pause $037F is over start the next state (delay 9, random count 1-4,
 * frame 8). */
void ObjectFlicker_m1x1(void)
{
    if (object_ready()) {
        lda8(rd8(ABSX(OBJ_FRAME)));
        eor8(0x01);
        wr8(ABSX(OBJ_FRAME), (u8)A);
        lda8(0x0A); wr8(ABSX(OBJ_DELAY), 0x0A);
    }
    lda8(rd8(ABSX(OBJ_COUNTER)));
    if (fz) {
        lda8(0x0A); wr8(ABSX(OBJ_DELAY), 0x0A);
        lda8(0x09); wr8(ABSX(OBJ_DELAY), 0x09);
        push8(0x81); push16(0xBE7A); RandomB_m1x1();
        and8(0x03);
        lda8(inc8((u8)A));
        wr8(ABSX(OBJ_RANDOM), (u8)A);
        wr8(ABSX(OBJ_STEP), 0);
        wr8(ABSX(OBJ_STATE), inc8(rd8(ABSX(OBJ_STATE))));
        lda8(0x08); wr8(ABSX(OBJ_FRAME), 0x08);
    }
    ret();
}
/* $80:F724 SpinWheels (RTS, riders X..0), $81:C579 SpinWheels8 (RTS,
 * riders 7..0). */
void SpinWheels_m1x1(void)  { spin_wheels(); ret(); }
void SpinWheels8_m1x1(void) { ldx8(0x07); spin_wheels(); ret(); }
/* $80:B73F EndJumpMode (RTS): once the frame count reaches $0708, before
 * broadcast minute $28 go to the course preview (mode $18, course 2),
 * from it on to mode $10; $86/$87 = 0 (JMP $80:8F17). */
void EndJumpMode_m1x1(void)
{
    rep(0x20);
    lda16(rd16(LONG(BCAST_FRAME)));
    cmp16(A, 0x0708);
    sep(0x20);
    if (!fc) { ret(); return; }
    lda8(rd8(LONG(BCAST_MINUTE)));
    cmp8((u8)A, 0x28);
    if (!fc) {
        lda8(0x18); wr8(DP(GAME_MODE), 0x18);
        wr8(DP(UPLOAD_SRC), 0);
        wr8(DP(0x87), 0);
        lda8(0x02); wr8(DP(COURSE_SLOT), 0x02);
        wr8(DP(FINAL_RACE), 0);
    } else {
        wr8(DP(UPLOAD_SRC), 0);
        wr8(DP(0x87), 0);
        lda8(0x10); wr8(DP(GAME_MODE), 0x10);
    }
    r816_call_pc(0x808F15);
}
/* $80:FED9 QueueBonusDigits (RTS): six digits $0CE5-$0CEA as a 6x2 tile
 * block at $0CEB (leading zeros blank $FF, the last digit always shown;
 * the lower row is digit | $10), size $26, then VramQueueTiles. */
void QueueBonusDigits_m1x1(void)
{
    ldx8(0x00);
    Y = X; nz8((u8)Y);
    do {
        lda8(rd8(ABSX(SCORE_DIGITS)));
        if (fz && Y == 0 && X != 5) {
            lda8(0xFF);
        } else {
            Y = inc8((u8)Y);
            lda8(rd8(ABSX(SCORE_DIGITS)));
        }
        wr8(ABSX(0x0CEC), (u8)A);
        ora8(0x10);
        wr8(ABSX(0x0CF2), (u8)A);
        X = inc8((u8)X);
        cmp8((u8)X, 0x06);
    } while (!fc);
    lda8(0x26); wr8(ABS(0x0CEB), 0x26);
    lda8(0xEB); wr8(DP(0x02), 0xEB);
    lda8(0x0C); wr8(DP(0x03), 0x0C);
    VramQueueTiles_m1x1();
}
/* $82:811D SetupHdmaChannels (RTL): HDMA channel 7 to register $0F (BG2
 * horizontal scroll) from the table at $82:815A and channel 6 to $0D (BG1)
 * from $82:8176, both mode $42 (indirect, two writes once), data bank 0. */
void SetupHdmaChannels_m1x1(void)
{
    lda8(0x42); wr8(ABS(DMAP7), 0x42);
    lda8(0x0F); wr8(ABS(BBAD7), 0x0F);
    lda8(0x5A); wr8(ABS(A1T7L), 0x5A);
    lda8(0x81); wr8(ABS(A1T7H), 0x81);
    lda8(0x82); wr8(ABS(A1B7), 0x82);
    lda8(0x00); wr8(ABS(DASB7), 0x00);
    lda8(0x42); wr8(ABS(DMAP6), 0x42);
    lda8(0x0D); wr8(ABS(BBAD6), 0x0D);
    lda8(0x76); wr8(ABS(A1T6L), 0x76);
    lda8(0x81); wr8(ABS(A1T6H), 0x81);
    lda8(0x82); wr8(ABS(A1B6), 0x82);
    lda8(0x00); wr8(ABS(DASB6), 0x00);
    retl();
}
/* $80:B062 RiderAhead (RTS): is another rider (from $7E up to 7, not $0F)
 * in the air state 1 ($019E), in lane $02 ($0958) and between
 * $0196,X + table[Y] - table[Y+1] and $0196,X + table[Y] (tables at
 * $80:B88B)? C = 1 with Y = that rider, else C = 0. */
void RiderAhead_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_X)));
    fc = 0;
    adc8(rd8(ABSY(0xB894)));
    wr8(DP(0x00), (u8)A);
    fc = 1;
    sbc8(rd8(ABSY(0xB895)));
    wr8(DP(0x01), (u8)A);
    Y = rd8(DP(0x7E)); nz8((u8)Y);
    do {
        cmp8((u8)Y, rd8(DP(0x0F)));
        if (fz) goto next;
        lda8(rd8(ABSY(RIDER_PAGE)));
        cmp8((u8)A, 0x01);
        if (!fz) goto next;
        lda8(rd8(ABSY(RIDER_BAND)));
        if (fz) goto next;
        cmp8((u8)A, rd8(DP(0x02)));
        if (!fz) goto next;
        lda8(rd8(ABSY(RIDER_X)));
        cmp8((u8)A, rd8(DP(0x00)));
        if (fc) goto next;
        cmp8((u8)A, rd8(DP(0x01)));
        if (fc) { ret(); return; }
    next:
        Y = inc8((u8)Y);
        cmp8((u8)Y, 0x08);
    } while (!fc);
    fc = 0;
    ret();
}
/* $80:D1EF DrawRiderMarker (RTL): sprite X + 1 (OAM buffer $1600) at
 * $1500,X / $1508,X with the blinking tile from $80:D218 (by $63 bits 2-3),
 * attribute $21, size bit 2. */
void DrawRiderMarker_m1x1(void)
{
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x0C);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xD221)));
    wr8(DP(0x00), (u8)A);
    lda8((u8)X);
    lda8(asl8((u8)A));
    fc = 0;
    adc8(0x04);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(DP(0x00))); wr8(ABSY(OAM_BUF + 0x02), (u8)A);
    lda8(rd8(ABSX(MARKER_X))); wr8(ABSY(OAM_BUF), (u8)A);
    lda8(rd8(ABSX(MARKER_Y))); wr8(ABSY(OAM_BUF + 0x01), (u8)A);
    lda8(0x21); wr8(ABSY(OAM_BUF + 0x03), 0x21);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02); wr8(ABSY(OAM_SIZE_BUF), 0x02);
    retl();
}
/* $81:9555 StepFallingSprite (RTS, object Y): every 4 frames move it up
 * and left ($0106, $010E) and step its tile $011E; at tile $9F switch it
 * off ($0116 = 0). While racing ($01CE and $84 clear) write it to the OAM
 * buffer $1610 + 2Y, attribute $2D. */
void StepFallingSprite_m1x1(void)
{
    X = Y; nz8((u8)X);
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x03);
    if (fz) {
        wr8(ABSX(EFFECT_X), dec8(rd8(ABSX(EFFECT_X))));
        wr8(ABSX(EFFECT_Y), dec8(rd8(ABSX(EFFECT_Y))));
        wr8(ABSX(EFFECT_FRAME), dec8(rd8(ABSX(EFFECT_FRAME))));
        lda8(rd8(ABSX(EFFECT_FRAME)));
        cmp8((u8)A, 0x9F);
        if (fz) { wr8(ABSX(EFFECT_KIND), 0); ret(); return; }
    }
    lda8(rd8(ABS(RIDER_BUMPED)));
    ora8(rd8(DP(RACE_END)));
    if (fz) {
        lda8((u8)X);
        lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSX(EFFECT_X))); wr8(ABSY(OAM_BUF + 0x10), (u8)A);
        lda8(rd8(ABSX(EFFECT_Y))); wr8(ABSY(OAM_BUF + 0x11), (u8)A);
        lda8(rd8(ABSX(EFFECT_FRAME))); wr8(ABSY(OAM_BUF + 0x12), (u8)A);
        lda8(0x2D); wr8(ABSY(OAM_BUF + 0x13), 0x2D);
    }
    ret();
}
/* $80:EB8C DrawCountdownLights (RTS): while $C3 is on (until the timer
 * $8E runs out) draw six sprites in a row at X $50 + 16n, Y $50, tiles
 * from $80:EBB6, attribute $31. */
void DrawCountdownLights_m1x1(void)
{
    lda8(rd8(DP(COUNTDOWN)));
    if (fz) { ret(); return; }
    u8 t = dec8(rd8(DP(LIGHTS_TIMER)));
    wr8(DP(LIGHTS_TIMER), t);
    if (fz) wr8(DP(COUNTDOWN), 0);
    push8((u8)X);
    ldx8(0x05);
    do {
        lda8((u8)X);
        lda8(asl8((u8)A));
        lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(asl8((u8)A));
        lda8(asl8((u8)A));
        fc = 0;
        adc8(0x50);
        wr8(ABSY(OAM_BUF), (u8)A);
        lda8(0x50); wr8(ABSY(OAM_BUF + 0x01), 0x50);
        lda8(rd8(ABSX(0xEBBF))); wr8(ABSY(OAM_BUF + 0x02), (u8)A);
        lda8(0x31); wr8(ABSY(OAM_BUF + 0x03), 0x31);
        lda8(0x02); wr8(ABSX(OAM_SIZE_BUF), 0x02);
        X = dec8((u8)X);
    } while (!fn);
    X = pull8(); nz8((u8)X);
    ret();
}
/* $81:BE8E ObjectRepeat4Rest (RTS): count the step 0-3; each time it
 * wraps count $032F down; at zero rest in state 2 with delay 10 and
 * frame 6. */
void ObjectRepeat4Rest_m1x1(void)
{
    if (object_ready()) {
        lda8(rd8(ABSX(OBJ_STEP)));
        lda8(inc8((u8)A));
        and8(0x03);
        wr8(ABSX(OBJ_STEP), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        int rest = 0;
        if (fz) {
            u8 n = dec8(rd8(ABSX(OBJ_RANDOM)));
            wr8(ABSX(OBJ_RANDOM), n);
            rest = fz;
        }
        if (rest) {
            lda8(0x02); wr8(ABSX(OBJ_STATE), 0x02);
            lda8(0x0A); wr8(ABSX(OBJ_DELAY), 0x0A);
            lda8(0x06); wr8(ABSX(OBJ_FRAME), 0x06);
            random_rest(0xBEB9);
            wr8(ABSX(OBJ_COUNTER), (u8)A);
        } else {
            object_frame(0xBED0, 0xBED4);
        }
    }
    ret();
}
/* $80:CE28 SortRidersByPosition (RTS): $0950-$0957 = rider numbers sorted
 * by their position $01F6 (the player uses $097C when $08C7 is set),
 * largest first (bubble sort through $00-$07). */
void SortRidersByPosition_m1x1(void)
{
    ldx8(0x07);
    do {
        lda8((u8)X);
        wr8(ABSX(RANK_ORDER), (u8)A);
        lda8(rd8(ABSX(RIDER_LANE)));
        wr8(DPX(0x00), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    lda8(rd8(ABS(BALL_OUT)));
    if (!fz) {
        lda8(rd8(ABS(BALL_LANE)));
        wr8(DP(0x03), (u8)A);
    }
    ldx8(0x07);
    wr8(DP(0x0D), 0x07);
    do {
        ldy8(0x00);
        do {
            X = rd8(ABSY(RANK_ORDER)); nz8((u8)X);
            lda8(rd8(DPX(0x00)));
            X = rd8(ABSY(RANK_ORDER + 0x01)); nz8((u8)X);
            cmp8((u8)A, rd8(DPX(0x00)));
            if (fc) {
                lda8(rd8(ABSY(RANK_ORDER)));
                wr8(ABSY(RANK_ORDER + 0x01), (u8)A);
                lda8((u8)X);
                wr8(ABSY(RANK_ORDER), (u8)A);
            }
            Y = inc8((u8)Y);
            cmp8((u8)Y, rd8(DP(0x0D)));
        } while (!fz);
        u8 n = dec8(rd8(DP(0x0D)));
        wr8(DP(0x0D), n);
    } while (!fz);
    ret();
}
/* $80:DDB7 QueueResultsCount (RTS): the number $10 as two digits (tens
 * blank when zero) in a 2x2 block at $03CF (size $22), queued at VRAM
 * $18B9 (VramQueueTiles20). */
void QueueResultsCount_m1x1(void)
{
    rep(0x20);
    lda16(0x18B9); wr16(DP(0x00), 0x18B9);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x22); wr8(ABS(DIGIT_BUF), 0x22);
    ldy8(0x00);
    lda8(rd8(DP(0x10)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
        WAIT_TICK();
    }
    wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
    ora8(0x10);
    wr8(ABS(DIGIT_ROW + 0x03), (u8)A);
    lda8((u8)Y);
    wr8(ABS(DIGIT_ROW), (u8)A);
    ora8(0x10);
    wr8(ABS(DIGIT_ROW + 0x02), (u8)A);
    cmp8((u8)Y, 0x00);
    if (fz) {
        lda8(0xFF);
        wr8(ABS(DIGIT_ROW), 0xFF);
        wr8(ABS(DIGIT_ROW + 0x02), 0xFF);
    }
    push16(0xDDF6); VramQueueTiles20_m1x1();
    ret();
}
/* $80:AA9D, $80:D914 DrawCountdownSign/B (RTS): tables $80:AAC8, $80:D948. */
void DrawCountdownSign_m1x1(void)  { draw_countdown_sign(0xAAD1); ret(); }
void DrawCountdownSignB_m1x1(void) { draw_countdown_sign(0xD951); ret(); }
/* $80:C13A StartCrash (RTS): rider X falls (state $01EE = 2, timer $08E4
 * = $0F) with a bounce speed $08D4/$08DC = its speed + $AF (halved when
 * $08F4 is 2). For the player also clear $09C7 and play sound $20. */
void StartCrash_m1x1(void)
{
    lda8(0x02); wr8(ABSX(RIDER_AIR), 0x02);
    lda8(0x0F); wr8(ABSX(RIDER_HEIGHT_LO), 0x0F);
    lda8(rd8(ABSX(RIDER_SPEED_LO)));
    fc = 0;
    adc8(0xAF);
    wr8(ABSX(RIDER_BOUNCE_LO), (u8)A);
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    adc8(0x00);
    wr8(ABSX(RIDER_BOUNCE_HI), (u8)A);
    lda8(rd8(ABSX(RIDER_SLOW)));
    cmp8((u8)A, 0x02);
    if (fz) {
        wr8(ABSX(RIDER_BOUNCE_HI), lsr8(rd8(ABSX(RIDER_BOUNCE_HI))));
        wr8(ABSX(RIDER_BOUNCE_LO), ror8(rd8(ABSX(RIDER_BOUNCE_LO))));
    }
    lda8((u8)X);
    if (!fz) { ret(); return; }
    wr8(ABS(0x09C7), (u8)A);
    lda8(0x20);
    push8(0x80); push16(0xC16D); QueueSoundB_m1x1();
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    eor8(0x30);
    if (fz) lda8(0x08);
    ret();
}
/* $80:DCEF AddBonusPoints (RTS): when the result $09C5 (forced 0 when
 * $1990 >= 2) equals the target $09C9 add 1 to $1E92 ($09AD = 1); else
 * split it into tens $09AE / units $09AF and add them to $1E93 / $1E94.
 * Then AddScore. */
void AddBonusPoints_m1x1(void)
{
    lda8(rd8(ABS(BONUS_END)));
    lda8(lsr8((u8)A));
    if (!fz) wr8(ABS(BONUS_RESULT), 0);
    wr8(DP(0x00), 0);
    lda8(rd8(ABS(BONUS_RESULT)));
    cmp8((u8)A, rd8(ABS(0x09C9)));
    if (fz) {
        lda8(0x01); wr8(ABS(SCORE_ADD + 0x01), 0x01);
        fc = 0;
        adc8(rd8(ABS(0x1E92)));
        wr8(ABS(0x1E92), (u8)A);
    } else {
        for (;;) {
            cmp8((u8)A, 0x0A);
            if (!fc) break;
            sbc8(0x0A);
            wr8(DP(0x00), inc8(rd8(DP(0x00))));
            WAIT_TICK();
        }
        wr8(ABS(SCORE_ADD + 0x03), (u8)A);
        fc = 0;
        adc8(rd8(ABS(0x1E94)));
        wr8(ABS(0x1E94), (u8)A);
        lda8(rd8(DP(0x00)));
        wr8(ABS(SCORE_ADD + 0x02), (u8)A);
        fc = 0;
        adc8(rd8(ABS(0x1E93)));
        wr8(ABS(0x1E93), (u8)A);
    }
    push16(0xDD32); AddScore_m1x1();
    ret();
}
/* $80:C060 SetRiderLean (RTS): rider X's lean $08B4 from the table at
 * $80:BA92 by its pose $01A6 ($0A for the player standing still on the
 * jump with $84 = 1; 6 while $020E,X is 1). */
void SetRiderLean_m1x1(void)
{
    Y = rd8(ABSX(RIDER_POSE)); nz8((u8)Y);
    lda8(rd8(DP(RACE_END)));
    cmp8((u8)A, 0x01);
    int table = 1;
    if (fz) {
        lda8((u8)X);
        ora8(rd8(ABS(RIDER_BUMPED)));
        ora8(rd8(ABS(RIDER_TIMER)));
        ora8(rd8(ABS(RIDER_SCRIPT)));
        if (fz) { lda8(0x0A); table = 0; }
    }
    if (table) lda8(rd8(ABSY(0xBA9B)));
    wr8(ABSX(RIDER_PIECE_LEAN), (u8)A);
    lda8(rd8(ABSX(RIDER_JUMP)));
    cmp8((u8)A, 0x01);
    if (fz) { lda8(0x06); wr8(ABSX(RIDER_PIECE_LEAN), 0x06); }
    ret();
}
/* $80:CFA6 QueuePlaceBadge (RTS): the place $086E as a 2x2 block at $0CF8
 * (digit tiles $60 + 2(place - 1), suffix tiles from $80:CFE2), size $24,
 * then VramQueueTiles. */
void QueuePlaceBadge_m1x1(void)
{
    lda8(rd8(ABS(PLAYER_PLACE)));
    lda8(dec8((u8)A));
    lda8(asl8((u8)A));
    push8((u8)A);
    fc = 0;
    adc8(0x60);
    wr8(ABS(0x0CF9), (u8)A);
    lda8(inc8((u8)A));
    wr8(ABS(0x0CFA), (u8)A);
    fc = 0;
    adc8(0x0F);
    wr8(ABS(0x0CFD), (u8)A);
    lda8(inc8((u8)A));
    wr8(ABS(0x0CFE), (u8)A);
    lda8(pull8());
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0xCFEB))); wr8(ABS(0x0CFB), (u8)A);
    lda8(rd8(ABSX(0xCFEC))); wr8(ABS(0x0CFC), (u8)A);
    lda8(rd8(ABSX(0xCFED))); wr8(ABS(0x0CFF), (u8)A);
    lda8(rd8(ABSX(0xCFEE))); wr8(ABS(0x0D00), (u8)A);
    lda8(0x24); wr8(ABS(0x0CF8), 0x24);
    lda8(0xF8); wr8(DP(0x02), 0xF8);
    lda8(0x0C); wr8(DP(0x03), 0x0C);
    VramQueueTiles_m1x1();
}
/* $80:F1A3 SetJumpScroll (RTL): from the player's height $01B6 and
 * 2 - $0904: below $0130 the vertical scroll $9E = value - $30, $A2 =
 * (value - $0130) / 2 + $8100 and $9B = $9E low; otherwise $9E = $A2 =
 * $0100, $9B = 0. */
void SetJumpScroll_m1x1(void)
{
    lda8(rd8(ABS(0x0904)));
    eor8(0xFF);
    lda8(inc8((u8)A));
    lda8(inc8((u8)A));
    wr8(DP(0x01), (u8)A);
    lda8(rd8(ABS(RIDER_Y)));
    wr8(DP(0x00), (u8)A);
    rep(0x20);
    lda16(rd16(DP(0x00)));
    cmp16(A, 0x0130);
    if (fc) {
        lda16(0x0100);
        wr16(DP(BG1_VSCROLL), 0x0100);
        wr16(DP(ZOOM), 0x0100);
        sep(0x20);
        wr8(DP(0x9B), 0);
        retl();
        return;
    }
    fc = 1;
    sbc16(0x0130);
    push16(A);
    fc = 0;
    adc16(0x0100);
    wr16(DP(BG1_VSCROLL), A);
    lda16(pull16());
    lda16(lsr16(A));
    fc = 0;
    adc16(0x8100);
    wr16(DP(ZOOM), A);
    sep(0x20);
    lda8(rd8(DP(BG1_VSCROLL)));
    wr8(DP(0x9B), (u8)A);
    retl();
}
/* $81:96F7 DrawSpark (RTS, object Y): sprite at OAM buffer $1610,Y from
 * $0106/$010E, tile and attribute from $81:966F/$81:9673 by the frame
 * $011E (frame 0 once $0146 reaches $F0); every $0126 frames restart the
 * frame ($012E = 1, $0126 = 4). */
void DrawSpark_m1x1(void)
{
    X = Y; nz8((u8)X);
    lda8(rd8(ABSX(EFFECT_X))); wr8(ABSY(OAM_BUF + 0x10), (u8)A);
    lda8(rd8(ABSX(EFFECT_Y))); wr8(ABSY(OAM_BUF + 0x11), (u8)A);
    lda8(rd8(ABSX(EFFECT_VY)));
    cmp8((u8)A, 0xF0);
    if (fc) lda8(0x00);
    else lda8(rd8(ABSX(EFFECT_FRAME)));
    push8((u8)X);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(LONGX(0x81966F))); wr8(ABSY(OAM_BUF + 0x12), (u8)A);
    lda8(rd8(LONGX(0x819673))); wr8(ABSY(OAM_BUF + 0x13), (u8)A);
    X = pull8(); nz8((u8)X);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02); wr8(ABSY(OAM_SIZE_BUF + 0x04), 0x02);
    u8 t = dec8(rd8(ABSX(EFFECT_TIMER)));
    wr8(ABSX(EFFECT_TIMER), t);
    if (fz) {
        lda8(0x01); wr8(ABSX(EFFECT_STEP), 0x01);
        lda8(0x04); wr8(ABSX(EFFECT_TIMER), 0x04);
        wr8(ABSX(EFFECT_FRAME), 0);
    }
    ret();
}
/* $80:9D45 StartPracticeMode (RTS): BG1SC = $1900 = 1, BG2SC 9, $1905 =
 * $17, $86/$87 = 0, the HDMA scroll channels, BG3SC $2B, BG12NBA $44,
 * BG34NBA 1, step $1923 and $75, timer $88 = $0708, $190D = $E3. */
void StartPracticeMode_m1x1(void)
{
    lda8(0x01); wr8(ABS(BG1SC), 0x01); wr8(ABS(0x1900), 0x01);
    lda8(0x09); wr8(ABS(BG2SC), 0x09);
    lda8(0x17); wr8(ABS(SH_TM), 0x17);
    wr8(DP(UPLOAD_SRC), 0);
    wr8(DP(0x87), 0);
    push8(0x80); push16(0x9D5E); SetupHdmaChannels_m1x1();
    lda8(0x2B); wr8(ABS(BG3SC), 0x2B);
    lda8(0x44); wr8(ABS(BG12NBA), 0x44);
    lda8(0x01); wr8(ABS(BG34NBA), 0x01);
    wr8(ABS(FADE_STATE), inc8(rd8(ABS(FADE_STATE))));
    wr8(DP(MODE_ACTIVE), inc8(rd8(DP(MODE_ACTIVE))));
    rep(0x20);
    lda16(0x1518); wr16(DP(MODE_TIMER), 0x1518);
    sep(0x20);
    lda8(0xE3); wr8(ABS(SH_CGADSUB), 0xE3);
    ret();
}
/* $80:E77A DrawJumpRank (RTS): in jump state 2 ($C7) show the rank sprite
 * (tile from $80:E7BB by $156B, negative ranks as 0) at X $38: it rises
 * from Y $A0 to $80, then stays for $40 frames ($8E). */
void DrawJumpRank_m1x1(void)
{
    lda8(rd8(DP(JUMP_STATE)));
    cmp8((u8)A, 0x02);
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0xD2)));
    if (fz) {
        lda8(rd8(ABS(JUMP_RANK)));
        if (fn) wr8(ABS(JUMP_RANK), 0);
        wr8(DP(0xD2), inc8(rd8(DP(0xD2))));
        lda8(0xA0); wr8(DP(0xD3), 0xA0);
        lda8(0x40); wr8(DP(LIGHTS_TIMER), 0x40);
    }
    lda8(rd8(DP(0xD3)));
    cmp8((u8)A, 0x80);
    if (fc) {
        wr8(DP(0xD3), dec8(rd8(DP(0xD3))));
    } else {
        lda8(rd8(DP(LIGHTS_TIMER)));
        if (fz) { ret(); return; }
        wr8(DP(LIGHTS_TIMER), dec8(rd8(DP(LIGHTS_TIMER))));
    }
    lda8(0x38); wr8(ABS(OAM_BUF + 0x100), 0x38);
    lda8(rd8(DP(0xD3))); wr8(ABS(OAM_BUF + 0x101), (u8)A);
    Y = rd8(ABS(JUMP_RANK)); nz8((u8)Y);
    lda8(rd8(ABSY(0xE7C4))); wr8(ABS(OAM_BUF + 0x102), (u8)A);
    lda8(0x21); wr8(ABS(OAM_BUF + 0x103), 0x21);
    lda8(0x02); wr8(ABS(OAM_SIZE_BUF + 0x40), 0x02);
    ret();
}
/* $82:8698 StepFade (RTL): brightness $1903 low nibble += fade step
 * $1923, clamped at $0F (ending the fade). Below zero the screen is dark:
 * reset the scroll, sound and race flags, play sound $80, $1903 = $80. */
void StepFade_m1x1(void)
{
    lda8(rd8(ABS(SH_INIDISP)));
    and8(0x0F);
    fc = 0;
    adc8(rd8(ABS(FADE_STATE)));
    if (fn) {
        wr8(DP(BG_SCROLL), 0); wr8(DP(0x9B), 0); wr8(DP(SCROLL_X), 0); wr8(DP(SCROLL_PAGE), 0);
        wr8(ABS(SPRITES_FROZEN), 0);
        wr8(ABS(ANIM_UPLOAD_ON), 0);
        lda8(0x80);
        push8(0x82); push16(0x86BE); QueueSoundB_m1x1();
        lda8(0x01); wr8(ABS(0x1E84), 0x01);
        lda8(0x80); wr8(ABS(SH_INIDISP), 0x80);
        wr8(ABS(APUIO2), 0);
        for (u16 a = 0x1961; a <= 0x1964; a++)
            wr8(ABS(a), 0);
        wr8(ABS(0x00CA), 0);
        wr8(ABS(UPLOAD_SRC), 0);
        wr8(ABS(0x0087), 0);
        wr8(ABS(ANIM_UPLOAD_ON), 0);
    } else {
        cmp8((u8)A, 0x0F);
        if (!fc) { wr8(ABS(SH_INIDISP), (u8)A); retl(); return; }
        lda8(0x0F);
    }
    wr8(ABS(FADE_STATE), 0);
    wr8(ABS(SH_INIDISP), (u8)A);
    retl();
}
/* $80:B7A9 Mode17_BonusIntro (RTS): count the word timer $88 down (sound
 * $34 at $06CC); below that zoom $A6 up by 4 to $0100, then set colour
 * math ($190D $41, COLDATA $E0) and wait for a press. At zero or on a
 * press leave the mode (JMP $80:8F15); otherwise $1905 = $17. */
void Mode17_BonusIntro_m1x1(void)
{
    rep(0x20);
    lda16(rd16(ABS(MODE_TIMER)));
    lda16(dec16(A));
    wr16(ABS(MODE_TIMER), A);
    if (fz) goto leave;
    cmp16(A, 0x06CC);
    if (fz) {
        sep(0x20);
        lda8(0x34);
        push8(0x80); push16(0xB7C4); QueueSoundB_m1x1();
    } else if (fc) {
        goto keep;
    }
    rep(0x20);
    lda16(rd16(DP(PIC_SCROLL)));
    cmp16(A, 0x0100);
    if (fz) {
        sep(0x20);
        lda8(0x41); wr8(ABS(SH_CGADSUB), 0x41);
        lda8(0xE0); wr8(ABS(COLDATA), 0xE0);
        lda8(rd8(DP(P1_PRESS_HI)));
        ora8(rd8(DP(P1_PRESS_LO)));
        if (!fz) goto leave;
        ret();
        return;
    }
    fc = 0;
    adc16(0x0004);
    wr16(DP(PIC_SCROLL), A);
keep:
    sep(0x20);
    lda8(0x17); wr8(ABS(SH_TM), 0x17);
    ret();
    return;
leave:
    sep(0x20);
    r816_call_pc(0x808F13);
}
/* $80:BC64 RiderAtStart (RTS): rider X at the start line ($3D,X clear):
 * riders other than the player are flagged as started ($01AE); once a
 * rider moves (or is off its start column $80:BACF) it is flagged too,
 * and riders from column 8 on (the player always) clear $01D6. A rival
 * not in the air also gets $199E = 1 and a random boost from $80:F6AB. */
void RiderAtStart_m1x1(void)
{
    lda8(rd8(DPX(RIDER_HOLD)));
    if (!fz) { ret(); return; }
    lda8((u8)X);
    if (!fz) { lda8(0x01); wr8(ABSX(RIDER_STEER), 0x01); }
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    ora8(rd8(ABSX(RIDER_STEER)));
    if (fz) {
        lda8(rd8(ABSX(RIDER_LANE)));
        cmp8((u8)A, rd8(ABSX(0xBAD8)));
        if (fz) { ret(); return; }
    }
    lda8(0x01); wr8(ABSX(RIDER_STEER), 0x01);
    lda8((u8)X);
    if (!fz) {
        lda8(rd8(ABSX(RIDER_LANE)));
        cmp8((u8)A, 0x08);
        if (!fc) { ret(); return; }
    }
    lda8(0x00); wr8(ABSX(RIDER_TIMER), 0x00);
    cmp8((u8)X, 0x00);
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_PAGE)));
    cmp8((u8)A, 0x01);
    if (fz) { ret(); return; }
    lda8(0x01); wr8(ABSX(0x199E), 0x01);
    push8(0x80); push16(0xBCA7); RandomB_m1x1();
    and8(0x03);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xF678))); wr8(ABSX(0x19A6), (u8)A);
    ret();
}
/* $81:9677 DrawStar (RTS, object Y): sprite at OAM buffer $1610 + 2Y
 * from $0106/$010E, tile from $81:96BF by frame $011E, attribute $2B;
 * every 4 frames step the frame, and after frame 2 move on ($012E + 1)
 * with a $40-frame wait. */
void DrawStar_m1x1(void)
{
    X = Y; nz8((u8)X);
    lda8((u8)X);
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSX(EFFECT_X))); wr8(ABSY(OAM_BUF + 0x10), (u8)A);
    lda8(rd8(ABSX(EFFECT_Y))); wr8(ABSY(OAM_BUF + 0x11), (u8)A);
    lda8(rd8(ABSX(EFFECT_FRAME)));
    push8((u8)X);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(LONGX(0x8196BF))); wr8(ABSY(OAM_BUF + 0x12), (u8)A);
    lda8(0x2B); wr8(ABSY(OAM_BUF + 0x13), 0x2B);
    X = pull8(); nz8((u8)X);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02); wr8(ABSY(OAM_SIZE_BUF + 0x04), 0x02);
    u8 t = dec8(rd8(ABSX(EFFECT_TIMER)));
    wr8(ABSX(EFFECT_TIMER), t);
    if (fz) {
        lda8(0x04); wr8(ABSX(EFFECT_TIMER), 0x04);
        wr8(ABSX(EFFECT_FRAME), inc8(rd8(ABSX(EFFECT_FRAME))));
        lda8(rd8(ABSX(EFFECT_FRAME)));
        cmp8((u8)A, 0x03);
        if (fc) {
            wr8(ABSX(EFFECT_STEP), inc8(rd8(ABSX(EFFECT_STEP))));
            lda8(0x40); wr8(ABSX(EFFECT_TIMER), 0x40);
        }
    }
    ret();
}
/* $81:9436 SpawnDustPuff (RTL): find a free effect slot (6, 4, 2 by
 * $0116) and, unless the dust table at $81:9476 says $80+ for frame $1961,
 * place a puff (tile $A7) at the player ($0196 / $01B6) offset by the
 * tables $81:9476 / $81:9496 minus 3. */
void SpawnDustPuff_m1x1(void)
{
    push8(DB);
    lda8(0x81);
    push8(0x81);
    DB = pull8(); nz8(DB);
    push8((u8)X);
    ldx8(0x06);
    do {
        lda8(rd8(ABSX(EFFECT_KIND)));
        if (fz) {
            Y = rd8(ABS(SPRITE_FRAME)); nz8((u8)Y);
            lda8(rd8(ABSY(0x9476)));
            if (!fn) {
                fc = 1; sbc8(0x03);
                fc = 0; adc8(rd8(ABS(RIDER_X)));
                wr8(ABSX(EFFECT_X), (u8)A);
                lda8(rd8(ABSY(0x9496)));
                fc = 1; sbc8(0x03);
                fc = 0; adc8(rd8(ABS(RIDER_Y)));
                wr8(ABSX(EFFECT_Y), (u8)A);
                lda8(0xA7); wr8(ABSX(EFFECT_FRAME), 0xA7);
                lda8(0x01); wr8(ABSX(EFFECT_KIND), 0x01);
            }
            X = pull8(); nz8((u8)X);
            DB = pull8(); nz8(DB);
            retl();
            return;
        }
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fz);
    X = pull8(); nz8((u8)X);
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:8FA4 ClearRaceState (RTS): zero $0884-$09B1, $09B2-$0CD3, $80-$84
 * and $197A-$19FF, with sound $2D in between. */
void ClearRaceState_m1x1(void)
{
    rep(0x10);
    for (X = 0x0884; X < 0x09B2; X++)
        wr8(DPX(0x00), 0);
    cmp16(X, 0x09B2);
    sep(0x10);
    lda8(0x09); wr8(DP(0x01), 0x09);
    lda8(0xB2); wr8(DP(0x00), 0xB2);
    rep(0x10);
    lda8(0x00);
    for (int y = 0x0321; y >= 0; y--)
        wr8((((u32)DB << 16) + rd16(DP(0x00)) + y) & 0xFFFFFF, 0);
    Y = 0xFFFF; nz16(Y);
    sep(0x10);
    ldx8(0x03);
    do {
        wr8(DPX(0x80), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    wr8(DP(RACE_END), (u8)A);
    lda8(0x2D);
    push8(0x80); push16(0x8FD7); QueueSoundB_m1x1();
    rep(0x10);
    lda8(0x7A); wr8(DP(0x00), 0x7A);
    lda8(0x19); wr8(DP(0x01), 0x19);
    lda8(0x00);
    for (int y = 0x0085; y >= 0; y--)
        wr8((((u32)DB << 16) + rd16(DP(0x00)) + y) & 0xFFFFFF, 0);
    Y = 0xFFFF; nz16(Y);
    sep(0x10);
    ret();
}
/* $80:A132 ReadBroadcastDate (RTL): merge the three BS-X bytes at
 * $11:572C-$11:572E as two-digit pairs into $0854-$0859. */
void ReadBroadcastDate_m1x1(void)
{
    or_digits(rd8(LONG(SAVE_BEST_TIME)), 0x0854, 0x0855);
    or_digits(rd8(LONG(SAVE_BEST_SEC)), 0x0856, 0x0857);
    or_digits(rd8(LONG(SAVE_BEST_FRAC)), 0x0858, 0x0859);
    retl();
}
/* $81:9744 MoveObjectX (RTL): add the signed velocity nibbles of $013E,X
 * (low nibble as fraction, high nibble as whole pixels) to the 8.8
 * position $014E/$0106-$0107. Returns A = carry + whole step. */
void MoveObjectX_m1x1(void)
{
    lda8(rd8(ABSX(EFFECT_VX)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    wr8(DP(0x01), (u8)A);
    lda8(rd8(ABSX(EFFECT_VX)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    cmp8((u8)A, 0x08);
    if (fc) ora8(0xF0);
    wr8(DP(0x00), (u8)A);
    ldy8(0x00);
    cmp8((u8)A, 0x00);
    if (fn) Y = dec8((u8)Y);
    wr8(DP(0x02), (u8)Y);
    lda8(rd8(ABSX(0x014E)));
    fc = 0;
    adc8(rd8(DP(0x01)));
    wr8(ABSX(0x014E), (u8)A);
    lda8(0x00);
    lda8(rol8((u8)A));
    push8((u8)A);
    lda8(ror8((u8)A));
    lda8(rd8(ABSX(EFFECT_X)));
    adc8(rd8(DP(0x00)));
    wr8(ABSX(EFFECT_X), (u8)A);
    lda8(rd8(ABSX(EFFECT_X + 0x01)));
    adc8(rd8(DP(0x02)));
    wr8(ABSX(EFFECT_X + 0x01), (u8)A);
    lda8(pull8());
    fc = 0;
    adc8(rd8(DP(0x00)));
    retl();
}
/* $80:C5B9 QueueRaceTime (RTS): the race clock $03AE-$03B0, then
 * VramQueueTiles. $80:C623 QueueBestTime (RTS): the record $083A-$083C,
 * then VramQueueTiles20. */
void QueueRaceTime_m1x1(void) { build_time(0x03AE, 0x03AF, 0x03B0, 0); VramQueueTiles_m1x1(); }
void QueueBestTime_m1x1(void) { build_time(0x083A, 0x083B, 0x083C, 1); VramQueueTiles20_m1x1(); }
/* $80:AE3C AccelerateRider (RTS): the player's ($7F) speed $01BE/$01C6
 * gains the step from $80:B864 (or $80:B86D in a race below page 2),
 * capped at $80:B883/$80:B886 for rider 0. A rider with $019E negative
 * is set to speed $0480 ($0580 when the player has not crashed). */
void AccelerateRider_m1x1(void)
{
    cmp8((u8)X, rd8(DP(RIDER_TURN)));
    if (fz) {
        lda8(rd8(DP(GAME_MODE)));
        cmp8((u8)A, 0x03);
        int alt = 0;
        if (fz) {
            lda8(rd8(DP(SCROLL_PAGE)));
            cmp8((u8)A, 0x02);
            alt = !fc;
        }
        lda8(rd8(ABSY(alt ? 0xB876 : 0xB86D)));
        fc = 0;
        adc8(rd8(ABSX(RIDER_SPEED_LO)));
        wr8(ABSX(RIDER_SPEED_LO), (u8)A);
        if (fc) wr8(ABSX(RIDER_SPEED_HI), inc8(rd8(ABSX(RIDER_SPEED_HI))));
        lda8((u8)X);
        if (fz) {
            lda8(rd8(ABSX(RIDER_SPEED_HI)));
            cmp8((u8)A, rd8(ABSY(0xB88F)));
            int cap = 0;
            if (fz) {
                lda8(rd8(ABSX(RIDER_SPEED_LO)));
                cmp8((u8)A, rd8(ABSY(0xB88C)));
                cap = fc;
            } else {
                cap = fc;
            }
            if (cap) {
                lda8(rd8(ABSY(0xB88C))); wr8(ABSX(RIDER_SPEED_LO), (u8)A);
                lda8(rd8(ABSY(0xB88F))); wr8(ABSX(RIDER_SPEED_HI), (u8)A);
            }
        }
    }
    lda8(rd8(ABSX(RIDER_PAGE)));
    if (fn) {
        lda8(rd8(ABS(RIDER_AIR)));
        lda8(fz ? 0x05 : 0x04);
        wr8(ABSX(RIDER_SPEED_HI), (u8)A);
        lda8(0x80); wr8(ABSX(RIDER_SPEED_LO), 0x80);
    }
    ret();
}
/* $80:BD91 UpdateRiderDistances (RTS): in a race ($7D, $92 set, $84
 * clear) the word distances $085E + 2X for riders 7..0: on-screen riders
 * ($01DE) from their position $0196/$019E + scroll $9C - $0100; others
 * add their speed $0D01 to the fraction $015E and the carry. */
void UpdateRiderDistances_m1x1(void)
{
    lda8(rd8(DP(FINAL_RACE)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(RACE_STARTED)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(RACE_END)));
    if (!fz) { ret(); return; }
    rep(0x20);
    ldy8(0x0E);
    ldx8(0x07);
    do {
        lda16(rd16(ABSX(RIDER_ACTIVE)));
        and16(0x00FF);
        if (!fz) {
            lda16(rd16(ABSX(RIDER_X))); wr16(DP(0x00), A);
            lda16(rd16(ABSX(RIDER_PAGE))); wr16(DP(0x01), A);
            lda16(rd16(DP(0x00)));
            fc = 0; adc16(rd16(DP(SCROLL_X)));
            fc = 1; sbc16(0x0100);
            wr16(ABSY(RIDER_DIST), A);
        } else {
            lda16(rd16(ABSY(0x015E)));
            fc = 0; adc16(rd16(ABSY(0x0D01)));
            push16(A);
            and16(0x00FF);
            wr16(ABSY(0x015E), A);
            lda16(pull16());
            A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
            and16(0x00FF);
            fc = 0; adc16(rd16(ABSY(RIDER_DIST)));
            wr16(ABSY(RIDER_DIST), A);
        }
        X = dec8((u8)X);
        Y = dec8((u8)Y);
        Y = dec8((u8)Y);
    } while (!fn);
    sep(0x20);
    ret();
}
/* $81:9592 DrawDustPair (RTS, object Y): scroll the pair left with the
 * player's speed ($0106 and $012E -= $0186); every 4 frames step the
 * tile $011E down, ending at $AF ($0116 = 0). While racing write both
 * sprites (OAM buffer $1610 + 2Y and $17E0 + 2Y, attribute $2D). */
void DrawDustPair_m1x1(void)
{
    X = Y; nz8((u8)X);
    lda8(rd8(ABSX(EFFECT_X)));
    fc = 1; sbc8(rd8(ABS(RIDER_STEP)));
    wr8(ABSX(EFFECT_X), (u8)A);
    lda8(rd8(ABSX(EFFECT_STEP)));
    fc = 1; sbc8(rd8(ABS(RIDER_STEP)));
    wr8(ABSX(EFFECT_STEP), (u8)A);
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x03);
    if (fz) {
        wr8(ABSX(EFFECT_FRAME), dec8(rd8(ABSX(EFFECT_FRAME))));
        lda8(rd8(ABSX(EFFECT_FRAME)));
        cmp8((u8)A, 0xAF);
        if (fz) { wr8(ABSX(EFFECT_KIND), 0); ret(); return; }
    }
    lda8(rd8(ABS(RIDER_BUMPED)));
    ora8(rd8(DP(RACE_END)));
    if (fz) {
        lda8((u8)X);
        lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSX(EFFECT_X))); wr8(ABSY(OAM_BUF + 0x10), (u8)A);
        lda8(rd8(ABSX(EFFECT_Y))); wr8(ABSY(OAM_BUF + 0x11), (u8)A);
        lda8(rd8(ABSX(EFFECT_FRAME))); wr8(ABSY(OAM_BUF + 0x12), (u8)A);
        lda8(0x2D); wr8(ABSY(OAM_BUF + 0x13), 0x2D);
        lda8(rd8(ABSX(EFFECT_STEP))); wr8(ABSY(OAM_BUF + 0x1E0), (u8)A);
        lda8(rd8(ABSX(0x0136))); wr8(ABSY(OAM_BUF + 0x1E1), (u8)A);
        lda8(rd8(ABSX(EFFECT_FRAME))); wr8(ABSY(OAM_BUF + 0x1E2), (u8)A);
        lda8(0x2D); wr8(ABSY(OAM_BUF + 0x1E3), 0x2D);
    }
    ret();
}
/* $80:ACDF RiderOnRamp (RTS): from the track piece $0216,X (table
 * $80:AD33) decide whether rider X is on a ramp ($08BC = 2) or not
 * ($08BC = 0, only from 1). Landing the player on a big ramp in the bonus
 * mode at height $B0 plays sound $46. For the player Z = not moving. */
void RiderOnRamp_m1x1(void)
{
    Y = rd8(ABSX(RIDER_TILE_Y)); nz8((u8)Y);
    lda8(rd8(ABSY(0xAD3C)));
    if (fz) {
        lda8(0x03);
        cmp8((u8)A, rd8(ABSX(RIDER_JUMP)));
        int ramp = 0;
        if (fz) {
            lda8(rd8(ABSX(RIDER_BAND)));
            cmp8((u8)A, 0x03);
            ramp = fc;
        }
        if (!ramp) {
            lda8(rd8(ABSX(RIDER_BAND)));
            if (!fz) {
                lda8(0x01);
                cmp8((u8)A, rd8(ABSX(RIDER_ON_RAMP)));
                if (fz) { ret(); return; }
                lda8(lsr8((u8)A));
                wr8(ABSX(RIDER_ON_RAMP), (u8)A);
                ret();
                return;
            }
        }
    }
    lda8(0x01);
    cmp8((u8)A, rd8(ABSX(RIDER_ON_RAMP)));
    if (!fz) {
        push8((u8)A);
        lda8(rd8(DP(GAME_MODE)));
        cmp8((u8)A, 0x09);
        if (fz) {
            lda8(rd8(ABSY(0xAD3C)));
            if (fn) {
                lda8(rd8(ABS(RIDER_ON_RAMP)));
                if (fz) {
                    lda8(rd8(ABS(RIDER_Y)));
                    cmp8((u8)A, 0xB0);
                    if (fz) {
                        lda8(0x46);
                        push8(0x80); push16(0xAD1E); QueueSoundB_m1x1();
                    }
                }
            }
        }
        lda8(pull8());
        lda8(asl8((u8)A));
        wr8(ABSX(RIDER_ON_RAMP), (u8)A);
    }
    lda8((u8)X);
    if (fz) {
        lda8(rd8(ABS(RIDER_SPEED_LO)));
        ora8(rd8(ABS(RIDER_SPEED_HI)));
    }
    ret();
}
/* $81:C9A7 BuildJumpScore (RTL): shift the score digits $03CF-$03DB right
 * by three, then put the jump count (Hundreds of $1569) with its label
 * tiles $4B/$4C and $5B/$5C in front; size $29. Zero distance blanks it. */
void BuildJumpScore_m1x1(void)
{
    ldy8(0x05);
    ldx8(0x0C);
    do {
        lda8(rd8(ABSX(DIGIT_BUF)));
        wr8(ABSX(DIGIT_ROW + 0x02), (u8)A);
        X = dec8((u8)X);
        Y = dec8((u8)Y);
    } while (!fn);
    push16(0xC9B7); Hundreds_m1x1();
    ldx8(0x07);
    lda8(0x4B); wr8(ABSX(DIGIT_BUF), 0x4B);
    lda8(inc8((u8)A)); wr8(ABSX(DIGIT_ROW), (u8)A);
    lda8((u8)Y); wr8(ABSX(DIGIT_ROW + 0x01), (u8)A);
    ldx8(0x10);
    lda8(0x5B); wr8(ABSX(DIGIT_BUF), 0x5B);
    lda8(inc8((u8)A)); wr8(ABSX(DIGIT_ROW), (u8)A);
    lda8((u8)Y);
    fc = 0; adc8(0x10);
    wr8(ABSX(DIGIT_ROW + 0x01), (u8)A);
    lda8(0x29); wr8(ABS(DIGIT_BUF), 0x29);
    rep(0x20);
    lda16(rd16(ABS(JUMP_COUNT)));
    if (fz) {
        ldx8(0xFF);
        wr8(ABS(DIGIT_ROW + 0x05), 0xFF); wr8(ABS(DIGIT_ROW + 0x06), 0xFF); wr8(ABS(DIGIT_ROW + 0x07), 0xFF);
        wr8(ABS(DIGIT_ROW + 0x0E), 0xFF); wr8(ABS(DIGIT_ROW + 0x0F), 0xFF); wr8(ABS(DIGIT_ROW + 0x10), 0xFF);
        X = inc8((u8)X);
        wr8(ABS(DIGIT_ROW + 0x08), (u8)X);
        ldx8(0x10);
        wr8(ABS(DIGIT_ROW + 0x11), 0x10);
    }
    sep(0x20);
    retl();
}
/* $80:B601 Mode09_BonusIntro (RTS): a pre-delay $1991 first; then, once
 * the broadcast minute reaches the table $80:B5F6 (by course $7C / 2),
 * go to mode 9 ($1923 - 1). Otherwise count $88 down (sound $34 at $06CC,
 * nothing above it) and zoom $A6 to $0100, then set colour math ($190D
 * $42, COLDATA $E0). */
void Mode09_BonusIntro_m1x1(void)
{
    rep(0x20);
    lda16(rd16(ABS(0x1991)));
    if (!fz) {
        wr16(ABS(0x1991), dec16(rd16(ABS(0x1991))));
    } else {
        sep(0x20);
        lda8(rd8(DP(COURSE_SLOT)));
        lda8(lsr8((u8)A));
        X = (u8)A; nz8((u8)X);
        lda8(rd8(LONG(BCAST_MINUTE)));
        cmp8((u8)A, rd8(ABSX(0xB5FF)));
        if (fc) {
            lda8(0x09); wr8(DP(GAME_MODE), 0x09);
            wr8(DP(MODE_ACTIVE), 0);
            wr8(DP(MODE_STEP), 0);
            wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
            sep(0x20);
            ret();
            return;
        }
    }
    rep(0x20);
    lda16(rd16(ABS(MODE_TIMER)));
    lda16(dec16(A));
    wr16(ABS(MODE_TIMER), A);
    if (fz) goto done;
    cmp16(A, 0x06CC);
    if (fz) {
        lda16(0x0034);
        push8(0x80); push16(0xB636); QueueSoundB_m0x1();
    } else if (fc) {
        goto done;
    }
    lda16(rd16(DP(PIC_SCROLL)));
    cmp16(A, 0x0100);
    if (fz) {
        sep(0x20);
        lda8(0x42); wr8(ABS(SH_CGADSUB), 0x42);
        lda8(0xE0); wr8(ABS(COLDATA), 0xE0);
    } else {
        fc = 0;
        adc16(0x0004);
        wr16(DP(PIC_SCROLL), A);
    }
done:
    sep(0x20);
    ret();
}
/* $80:FE60 TrackRiderPace (RTS): while racing, count frames $0995 and the
 * distance $0996 (+ speed); every $3C units of distance pick the pace band
 * from $80:F715 (frames, for X = 7..1) and in a race set the AI targets
 * $09B0/$09B1 from $80:F71D/$80:F725. Restart both after $46 frames. */
void TrackRiderPace_m1x1(void)
{
    lda8(rd8(ABS(ATTRACT_DEMO)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(RACE_STARTED)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(RACE_END)));
    if (!fz) { ret(); return; }
    wr8(ABS(PACE_FRAMES), inc8(rd8(ABS(PACE_FRAMES))));
    lda8(rd8(ABS(PACE_FRAMES)));
    cmp8((u8)A, 0x46);
    if (fc) {
        lda8(0x00);
        wr8(ABS(PACE_FRAMES), 0);
        wr8(ABS(PACE_DIST), 0);
        ret();
        return;
    }
    lda8(rd8(ABS(RIDER_STEP)));
    fc = 0;
    adc8(rd8(ABS(PACE_DIST)));
    wr8(ABS(PACE_DIST), (u8)A);
    cmp8((u8)A, 0x3C);
    if (!fc) { ret(); return; }
    lda8(rd8(ABS(PACE_DIST)));
    fc = 1;
    sbc8(0x3C);
    wr8(ABS(PACE_DIST), (u8)A);
    ldx8(0x07);
    do {
        lda8(rd8(ABS(PACE_FRAMES)));
        cmp8((u8)A, rd8(ABSX(0xF6E2)));
        if (!fc) wr8(DP(0x00), (u8)X);
        else wr8(DP(0x01), (u8)X);
        X = dec8((u8)X);
    } while (!fz);
    lda8(rd8(DP(FINAL_RACE)));
    if (!fz) {
        X = rd8(DP(0x00)); nz8((u8)X);
        lda8(rd8(ABSX(0xF6EA))); wr8(ABS(SCORE_ADD + 0x04), (u8)A);
        lda8(rd8(ABSX(0xF6F2))); wr8(ABS(SCORE_ADD + 0x05), (u8)A);
    }
    lda8(0x00);
    wr8(ABS(PACE_FRAMES), 0);
    wr8(ABS(PACE_DIST), 0);
    ret();
}
/* $82:8782 HideAllSprites (RTL): unless $1958 is set, move all 128
 * sprites of the OAM buffer off screen (Y $F0) and clear the size table
 * $1821-$189F. */
void HideAllSprites_m1x1(void)
{
    static const u16 rows[] = { 0x1601, 0x1681, 0x1701, 0x1781 };
    lda8(rd8(ABS(SPRITES_FROZEN)));
    if (!fz) { retl(); return; }
    ldx8(0x00);
    do {
        lda8(0xF0);
        for (int r = 0; r < 4; r++)
            for (int k = 0; k < 8; k++)
                wr8(ABSX(rows[r] + 4 * k), 0xF0);
        lda8((u8)X);
        fc = 0;
        adc8(0x20);
        X = (u8)A; nz8((u8)X);
    } while (!fn);
    lda8(0x00);
    ldy8(0x7F);
    do {
        wr8(ABSY(OAM_SIZE_BUF), 0);
        Y = dec8((u8)Y);
    } while (!fz);
    retl();
}
/* $81:C75E CheckLaneTile (RTL): for rider X without a lane flag ($01AE),
 * by its lane mode $08A4 (0, 2, 3, 5) look for the marker tiles at
 * $0887 + 4X or under it ($0216,X): $65/$6A or $67/$68 give $01AE = $FF,
 * $35/$6A or $37/$68 give 1. */
void CheckLaneTile_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_STEER)));
    if (!fz) { retl(); return; }
    lda8((u8)X);
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    u8 mode = rd8(ABSX(RIDER_LANE_NUM));
    lda8(mode);
    u8 tile, under, value;
    if (fz) { cmp8(mode, 0x02); tile = 0x65; under = 0x6A; value = 0xFF; }
    else {
        cmp8(mode, 0x02);
        if (fz) { tile = 0x67; under = 0x68; value = 0xFF; }
        else {
            cmp8(mode, 0x03);
            if (fz) { tile = 0x35; under = 0x6A; value = 0x01; }
            else {
                cmp8(mode, 0x05);
                if (!fz) { retl(); return; }
                tile = 0x37; under = 0x68; value = 0x01;
            }
        }
    }
    lda8(rd8(ABSY(PIECES_AHEAD + 0x03)));
    cmp8((u8)A, tile);
    if (!fz) {
        lda8(rd8(ABSX(RIDER_TILE_Y)));
        cmp8((u8)A, under);
        if (!fz) { retl(); return; }
    }
    lda8(value);
    wr8(ABSX(RIDER_STEER), value);
    retl();
}
/* $80:E7CA DrawJumpRiderSprites (RTS): for a moving rider X on a ramp
 * ($08BC) in the air state 1 past column $10: its two sprites
 * (SetJumpRiderSprites), blinking every 4 frames, palette from the track
 * piece (or $0A off the track, $0E above height $58), attribute | $61,
 * and the lower one hidden while the speed high byte is zero. */
void DrawJumpRiderSprites_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_ON_RAMP)));
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    ora8(rd8(ABSX(RIDER_SPEED_LO)));
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_HEIGHT)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_PAGE)));
    cmp8((u8)A, 0x01);
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_X)));
    cmp8((u8)A, 0x10);
    if (!fc) { ret(); return; }
    push16(0xE7EC); SetJumpRiderSprites_m1x1();
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x04);
    if (!fz) {
        lda8(0xF0); wr8(ABSY(OAM_BUF + 0xC2), 0xF0);
        lda8(0xE0); wr8(ABSY(OAM_BUF + 0xC6), 0xE0);
    }
    lda8(rd8(ABSX(RIDER_BAND)));
    if (fz) {
        lda8(0x0A); wr8(DP(0x02), 0x0A);
    } else {
        push8((u8)Y);
        Y = rd8(ABSX(RIDER_TILE_Y)); nz8((u8)Y);
        lda8(rd8(ABSY(0xAD3C)));
        Y = pull8(); nz8((u8)Y);
        wr8(DP(0x02), (u8)A);
        cmp8((u8)A, 0x00);
        if (fz) {
            lda8(0xF0);
            wr8(ABSY(OAM_BUF + 0xC1), 0xF0);
            wr8(ABSY(OAM_BUF + 0xC5), 0xF0);
        }
    }
    lda8(rd8(ABSX(RIDER_Y)));
    cmp8((u8)A, 0x58);
    if (!fc) lda8(0x0E);
    else lda8(rd8(DP(0x02)));
    ora8(0x61);
    wr8(ABSY(OAM_BUF + 0xC3), (u8)A);
    wr8(ABSY(OAM_BUF + 0xC7), (u8)A);
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    if (fz) { lda8(0xF8); wr8(ABSY(OAM_BUF + 0xC5), 0xF8); }
    ret();
}
/* $81:9A27 FadeSkyColours (RTL): every 8 frames move the four colours at
 * $1C38-$1C3F one step towards $6C00 / $0160 / $0009 (blue, green, red)
 * and flag the palette upload ($1915). */
void FadeSkyColours_m1x1(void)
{
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x07);
    if (fz) {
        ldx8(0x06);
        rep(0x20);
        do {
            fade_field(0x7C00, 0x6C00, 0x0400);
            fade_field(0x03E0, 0x0160, 0x0020);
            fade_field(0x001F, 0x0009, 0x0001);
            X = dec8((u8)X);
            X = dec8((u8)X);
        } while (!fn);
        sep(0x20);
    }
    wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
    retl();
}
/* $80:B234 DrawBigSprite (RTS): a 2x2 block of 16x16 sprites at OAM
 * buffer $1600 + $04 from X $00 / Y $01; tiles from $80:B296 for X < 4
 * (attribute bit 0 cleared) or from $02 (+2 each), attribute
 * ($0B << 1) | $20 | $03; all four large. */
void DrawBigSprite_m1x1(void)
{
    push8((u8)Y);
    Y = rd8(DP(0x04)); nz8((u8)Y);
    lda8(rd8(DP(0x00)));
    wr8(ABSY(OAM_BUF), (u8)A); wr8(ABSY(OAM_BUF + 0x08), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(OAM_BUF + 0x04), (u8)A); wr8(ABSY(OAM_BUF + 0x0C), (u8)A);
    lda8(rd8(DP(0x01)));
    wr8(ABSY(OAM_BUF + 0x01), (u8)A); wr8(ABSY(OAM_BUF + 0x05), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(OAM_BUF + 0x09), (u8)A); wr8(ABSY(OAM_BUF + 0x0D), (u8)A);
    cmp8((u8)X, 0x04);
    if (!fc) {
        wr8(DP(0x03), 0);
        lda8(rd8(ABSX(0xB29F)));
    } else {
        lda8(rd8(DP(0x02)));
    }
    wr8(ABSY(OAM_BUF + 0x02), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(OAM_BUF + 0x06), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(OAM_BUF + 0x0A), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(OAM_BUF + 0x0E), (u8)A);
    lda8(rd8(DP(0x0B)));
    lda8(asl8((u8)A));
    ora8(0x20);
    ora8(rd8(DP(0x03)));
    wr8(ABSY(OAM_BUF + 0x03), (u8)A); wr8(ABSY(OAM_BUF + 0x07), (u8)A);
    wr8(ABSY(OAM_BUF + 0x0B), (u8)A); wr8(ABSY(OAM_BUF + 0x0F), (u8)A);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02);
    wr8(ABSY(OAM_SIZE_BUF), 0x02); wr8(ABSY(OAM_SIZE_BUF + 0x01), 0x02);
    wr8(ABSY(OAM_SIZE_BUF + 0x02), 0x02); wr8(ABSY(OAM_SIZE_BUF + 0x03), 0x02);
    Y = pull8(); nz8((u8)Y);
    ret();
}
/* $80:D7B4 QueueBonusCount (RTS): "x NN" style counter at $03CF (size
 * $27, label tiles $2D $22 $26 $0E $46 and their lower halves), the
 * number $B4 as two digits, queued at VRAM $184D with attribute $20. */
void QueueBonusCount_m1x1(void)
{
    static const u8 label[5] = { 0x2D, 0x22, 0x26, 0x0E, 0x46 };
    rep(0x20);
    lda16(0x184D); wr16(DP(0x00), 0x184D);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x27); wr8(ABS(DIGIT_BUF), 0x27);
    for (int i = 0; i < 5; i++) {
        lda8(label[i]); wr8(ABS(DIGIT_ROW + i), label[i]);
        fc = 0; adc8(0x10);
        wr8(ABS(DIGIT_ROW + 0x07 + i), (u8)A);
    }
    ldy8(0x00);
    lda8(rd8(DP(BONUS_SECONDS)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
        WAIT_TICK();
    }
    wr8(ABS(DIGIT_ROW + 0x06), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x0D), (u8)A);
    lda8((u8)Y);
    wr8(ABS(DIGIT_ROW + 0x05), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x0C), (u8)A);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xD822); VramQueueTiles_m1x1();
    ret();
}
/* $81:94B6 SpawnExhaust (RTL): while the turbo is on ($0CE0) and the
 * exhaust table $81:9518 allows frame $1961, every 4 frames put a pair of
 * puffs (tile $B2) into a free slot (6, 4, 2) behind the player at random
 * offsets from $2C-$2F. */
void SpawnExhaust_m1x1(void)
{
    push8(DB);
    lda8(0x81);
    push8(0x81);
    DB = pull8(); nz8(DB);
    push8((u8)X);
    lda8(rd8(ABS(TURBO_ON)));
    if (fz) goto out;
    Y = rd8(ABS(SPRITE_FRAME)); nz8((u8)Y);
    lda8(rd8(ABSY(0x9518)));
    if (fz) goto out;
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x03);
    if (!fz) goto out;
    ldx8(0x06);
    do {
        lda8(rd8(ABSX(EFFECT_KIND)));
        if (fz) {
            lda8(rd8(DP(RANDOM_LFSR))); and8(0x07); fc = 0; adc8(0x38);
            wr8(ABSX(EFFECT_X), (u8)A);
            lda8(rd8(DP(0x2D))); and8(0x0F); fc = 0; adc8(rd8(ABS(RIDER_Y))); sbc8(0x02);
            wr8(ABSX(EFFECT_Y), (u8)A);
            lda8(rd8(DP(0x2E))); and8(0x07); fc = 0; adc8(0x3C);
            wr8(ABSX(EFFECT_STEP), (u8)A);
            lda8(rd8(DP(0x2F))); and8(0x0F); fc = 0; adc8(rd8(ABS(RIDER_Y))); sbc8(0x02);
            wr8(ABSX(0x0136), (u8)A);
            lda8(0xB2); wr8(ABSX(EFFECT_FRAME), 0xB2);
            lda8(0x02); wr8(ABSX(EFFECT_KIND), 0x02);
            goto out;
        }
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fz);
out:
    X = pull8(); nz8((u8)X);
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:BBD8 RiderAtFinish (RTS): rider X crossing the finish ($08C4):
 * slow the run-out ($090C - 1 every 16 frames, every frame for the player
 * on a new turn press) and every 8 frames step the wave pose $091C
 * (0, 1, 3, 4; sound $23 at 3 and when it wraps). Once past $0196 + 8 the
 * rider stops: $56 = 3, $01D6 + 1, $099E = 0; rider 7 also clears $092C
 * and $08C7. */
void RiderAtFinish_m1x1(void)
{
    lda8(rd8(ABSX(0x08C4)));
    if (fz) goto stop;
    lda8(rd8(DPX(RIDER_LEAN_TIMER)));
    if (!fz) { ret(); return; }
    lda8((u8)X);
    if (fz) {
        Y = rd8(DP(0x79)); nz8((u8)Y);
        lda8(rd8(ABS(RIDER_INPUT)));
        and8(0xC0);
        if (!fz) {
            wr8(DP(0x79), (u8)A);
            lda8((u8)Y);
            if (fz) {
                wr8(ABSX(RIDER_RUNOUT), dec8(rd8(ABSX(RIDER_RUNOUT))));
                goto pose;
            }
        }
        wr8(DP(0x79), (u8)A);
    }
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x0F);
    if (fz) wr8(ABSX(RIDER_RUNOUT), dec8(rd8(ABSX(RIDER_RUNOUT))));
pose:
    and8(0x07);
    if (fz) {
        lda8(rd8(ABSX(FINISH_POSE)));
        lda8(inc8((u8)A));
        cmp8((u8)A, 0x02);
        if (fz) lda8(inc8((u8)A));
        cmp8((u8)A, 0x05);
        if (fz) {
            lda8(0x23);
            push8(0x80); push16(0xBC22); QueueSoundB_m1x1();
            lda8(0x00);
            wr8(ABSX(FINISH_POSE), 0x00);
        } else {
            cmp8((u8)A, 0x03);
            if (fz) {
                wr8(ABSX(FINISH_POSE), (u8)A);
                lda8(0x23);
                push8(0x80); push16(0xBC1A); QueueSoundB_m1x1();
            } else {
                wr8(ABSX(FINISH_POSE), (u8)A);
            }
        }
    }
    lda8(rd8(ABSX(RIDER_RUNOUT)));
    fc = 1;
    sbc8(0x08);
    cmp8((u8)A, rd8(ABSX(RIDER_X)));
    if (fc) { ret(); return; }
stop:
    lda8(0x03); wr8(DPX(0x56), 0x03);
    wr8(ABSX(RIDER_TIMER), inc8(rd8(ABSX(RIDER_TIMER))));
    lda8(0x00); wr8(ABSX(0x099E), 0x00);
    cmp8((u8)X, 0x07);
    if (fz) {
        wr8(ABS(0x092C), 0);
        wr8(ABS(BALL_OUT), 0);
    }
    ret();
}
/* $80:F12C BuildScoreDigits (M=0, RTS): the word A as five decimal digits
 * at $03D0-$03D4 (leading zeros blank $FF), the lower row (+$10) at
 * $03D6-$03DA, $03D5 = 0, $03DB = $10, size $26 at $03CF. */
void BuildScoreDigits_m0x1(void)
{
    static const u16 pow10[3] = { 10000, 1000, 100 };
    for (int i = 0; i < 3; i++) {
        ldy8(0x00);
        for (;;) {
            cmp16(A, pow10[i]);
            if (!fc) break;
            sbc16(pow10[i]);
            Y = inc8((u8)Y);
            WAIT_TICK();
        }
        wr8(ABS(DIGIT_ROW + i), (u8)Y);
    }
    ldy8(0x00);
    sep(0x20);
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
        WAIT_TICK();
    }
    wr8(ABS(DIGIT_ROW + 0x03), (u8)Y);
    wr8(ABS(DIGIT_ROW + 0x04), (u8)A);
    wr8(ABS(DIGIT_ROW + 0x05), 0);
    lda8(0x10); wr8(ABS(DIGIT_ROW + 0x0B), 0x10);
    ldy8(0x00);
    wr8(DP(0x00), 0);
    do {
        lda8(rd8(ABSY(DIGIT_ROW)));
        if (fz && rd8(DP(0x00)) == 0) {
            lda8(rd8(DP(0x00)));
            lda8(0xFF);
            wr8(ABSY(DIGIT_ROW), 0xFF);
            wr8(ABSY(DIGIT_ROW + 0x06), 0xFF);
        } else {
            lda8(rd8(ABSY(DIGIT_ROW)));
            ora8(0x10);
            wr8(ABSY(DIGIT_ROW + 0x06), (u8)A);
            wr8(DP(0x00), inc8(rd8(DP(0x00))));
        }
        Y = inc8((u8)Y);
        cmp8((u8)Y, 0x05);
    } while (!fc);
    lda8(0x26); wr8(ABS(DIGIT_BUF), 0x26);
    ret();
}
/* $81:9A9B FadePalette (RTL): every 8 frames move the colours $1C00 +
 * $B9 down to 0 (skipping each palette's colour 0) one step towards the
 * targets at $81:9B15 / $81:9B2D / $81:9B45 (blue, green, red), and flag
 * the palette upload ($1915). */
void FadePalette_m1x1(void)
{
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x07);
    if (fz) {
        X = rd8(DP(0xB9)); nz8((u8)X);
        rep(0x20);
        do {
            lda16((u16)X);
            and16(0x0007);
            if (!fz) {
                fade_to(0x7C00, 0x9B15, 0x0400);
                fade_to(0x03E0, 0x9B2D, 0x0020);
                fade_to(0x001F, 0x9B45, 0x0001);
            }
            X = dec8((u8)X);
            X = dec8((u8)X);
        } while (!fn);
        sep(0x20);
    }
    wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
    retl();
}
/* $80:D01C MirrorJumpRider (RTS): copy the player's four sprites (OAM
 * buffer $1600 + $04) to $1700 + $04 as the shadow at X $090C, Y $90 -
 * $0979 (all large), then make the originals tiles $C8-$CE with
 * attribute bit 0 set. */
void MirrorJumpRider_m1x1(void)
{
    Y = rd8(ABS(0x0004)); nz8((u8)Y);
    ldx8(0x10);
    do {
        lda8(rd8(ABSY(OAM_BUF)));
        wr8(ABSY(OAM_BUF + 0x100), (u8)A);
        Y = inc8((u8)Y);
        X = dec8((u8)X);
    } while (!fz);
    Y = rd8(ABS(0x0004)); nz8((u8)Y);
    lda8(rd8(ABS(RIDER_RUNOUT)));
    wr8(ABSY(OAM_BUF + 0x100), (u8)A); wr8(ABSY(OAM_BUF + 0x108), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(OAM_BUF + 0x104), (u8)A); wr8(ABSY(OAM_BUF + 0x10C), (u8)A);
    lda8(0x90);
    fc = 1; sbc8(rd8(ABS(0x0979)));
    wr8(ABSY(OAM_BUF + 0x101), (u8)A); wr8(ABSY(OAM_BUF + 0x105), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(OAM_BUF + 0x109), (u8)A); wr8(ABSY(OAM_BUF + 0x10D), (u8)A);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    fc = 0; adc8(0x40);
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02);
    wr8(ABSY(OAM_SIZE_BUF), 0x02); wr8(ABSY(OAM_SIZE_BUF + 0x01), 0x02);
    wr8(ABSY(OAM_SIZE_BUF + 0x02), 0x02); wr8(ABSY(OAM_SIZE_BUF + 0x03), 0x02);
    Y = rd8(ABS(0x0004)); nz8((u8)Y);
    lda8(rd8(ABSY(OAM_BUF + 0x03)));
    ora8(0x01);
    wr8(ABSY(OAM_BUF + 0x03), (u8)A); wr8(ABSY(OAM_BUF + 0x07), (u8)A);
    wr8(ABSY(OAM_BUF + 0x0B), (u8)A); wr8(ABSY(OAM_BUF + 0x0F), (u8)A);
    lda8(0xC8); wr8(ABSY(OAM_BUF + 0x02), 0xC8);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(OAM_BUF + 0x06), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(OAM_BUF + 0x0A), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(OAM_BUF + 0x0E), (u8)A);
    ret();
}
/* $82:8701 PlaceMinuteMarker (RTL): in a race (mode 3, $0CD7 clear) when
 * the broadcast minute changes ($BF) to one of the eight in $82:8773,
 * find the next free stretch of the track buffer $7F:4000 after the
 * player ($027B + 3; a column $80 not preceded by tiles $17/$19/$1B/$03/
 * $04, wrapping at 9) and drop the item from $82:877B by lane mode $08A4
 * there; clear $B8. */
void PlaceMinuteMarker_m1x1(void)
{
    push8(DB);
    lda8(0x82);
    push8(0x82);
    DB = pull8(); nz8(DB);
    lda8(rd8(LONG(BCAST_MINUTE)));
    cmp8((u8)A, rd8(ABS(0x00BF)));
    if (fz) goto out;
    lda8(rd8(ABS(ATTRACT_DEMO)));
    if (!fz) goto out;
    lda8(rd8(DP(GAME_MODE)));
    cmp8((u8)A, 0x03);
    if (!fz) goto out;
    ldx8(0x07);
    lda8(rd8(LONG(BCAST_MINUTE)));
    for (;;) {
        cmp8((u8)A, rd8(ABSX(0x8773)));
        if (fz) break;
        X = dec8((u8)X);
        if (fn) { DB = pull8(); nz8(DB); retl(); return; }
    }
    rep(0x10);
    X = rd16(ABS(TRACK_READ_POS)); nz16(X);
    X = inc16(X); X = inc16(X); X = inc16(X);
    for (;;) {
        lda8(rd8(LONGX(TRACK_BUF)));
        cmp8((u8)A, 0x09);
        if (fz) { X = 0x0000; nz16(X); }
        X = inc16(X);
        cmp8((u8)A, 0x80);
        WAIT_TICK();
        if (!fz) continue;
        lda8(rd8(LONGX(0x7F3FFE)));
        cmp8((u8)A, 0x17); if (fz) continue;
        cmp8((u8)A, 0x19); if (fz) continue;
        cmp8((u8)A, 0x1B); if (fz) continue;
        cmp8((u8)A, 0x03); if (fz) continue;
        cmp8((u8)A, 0x04); if (fz) continue;
        break;
    }
    rep(0x20);
    lda16(rd16(ABS(RIDER_LANE_NUM)));
    and16(0x00FF);
    Y = A; nz16(Y);
    sep(0x20);
    lda8(rd8(ABSY(0x877B)));
    wr8(LONGX(0x7F3FFF), (u8)A);
    wr8(ABS(0x00B8), 0);
out:
    sep(0x10);
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:CB81 RiderPose (RTS): returns A = the sprite pose for rider X
 * ($0E = extra flags): a waving pose | $10 at the finish or start
 * ($01D6), the crash and fall poses $13-$15, or its lean $01E6. */
void RiderPose_m1x1(void)
{
    wr8(DP(0x0E), 0);
    lda8(rd8(ABSX(RIDER_TIMER)));
    int special = fz;
    if (!fz) { cmp8((u8)A, 0x05); special = fz; }
    if (!special) {
        ldy8(0x06);
        lda8(rd8(ABSX(0x099E)));
        if (fz) ldy8(0x02);
        lda8((u8)Y);
        Y = rd8(DPX(0x56)); nz8((u8)Y);
        if (!fz) { Y = dec8((u8)Y); lda8((u8)Y); }
        ora8(0x10);
        ret();
        return;
    }
    Y = rd8(DP(RACE_END)); nz8((u8)Y);
    Y = dec8((u8)Y);
    if (fz) {
        lda8(rd8(ABSX(RIDER_LEAN)));
        cmp8((u8)A, 0x0A);
        if (fz) { ldy8(0x02); wr8(DP(0x0E), 0x02); ret(); return; }
    }
    lda8(rd8(ABSX(RIDER_BUMPED)));
    ora8(rd8(ABSX(RIDER_AIR)));
    if (!fz) goto lean;
    lda8(rd8(ABSX(RIDER_SCRIPT)));
    if (!fz) {
        cmp8((u8)A, 0x0D);
        if (!fz) goto lean;
        lda8(rd8(ABSX(RIDER_LANE)));
        cmp8((u8)A, 0x20);
    }
    lda8(rd8(ABSX(RIDER_SPEED_LO)));
    ora8(rd8(ABSX(RIDER_SPEED_HI)));
    if (fz) {
        wr8(DP(0x0E), inc8(rd8(DP(0x0E))));
        goto lean;
    }
    lda8(rd8(ABSX(RIDER_LEAN)));
    cmp8((u8)A, 0x06);
    if (!fz) goto lean;
    ldy8(0x14);
    lda8(rd8(ABSX(RIDER_STEER)));
    if (!fz) {
        if (fn) Y = inc8((u8)Y);
        lda8((u8)Y);
        ret();
        return;
    }
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x02);
    if (!fz) { lda8(0x13); ret(); return; }
lean:
    lda8(rd8(ABSX(RIDER_LEAN)));
    ret();
}
/* $80:D985 ReadJumpTrack (RTS): for the player's wheel column $0266:
 * count landing points ($0A93 at each new column while airborne or
 * crashed) into $09C5, take the height $0A13 while on the ground, and the
 * tile $09D3 into $0216. Tiles below $28 end the course once ($099C, $84,
 * $1990 = 1, sound $2B); tile flags $0A53 turn the column into $3B tiles
 * (or $45 while airborne); tile $84 becomes $FC on the ground. */
void ReadJumpTrack_m1x1(void)
{
    X = rd8(ABS(RIDER_COLUMN)); nz8((u8)X);
    lda8(rd8(ABS(0x09C7)));
    ora8(rd8(ABS(RIDER_AIR)));
    if (!fz) {
        lda8(rd8(ABS(RIDER_COLUMN)));
        cmp8((u8)A, rd8(ABS(0x09C8)));
        if (!fz) {
            wr8(ABS(0x09C8), (u8)A);
            lda8(rd8(ABSX(TRACK_ROWS + 0xC0)));
            fc = 0; adc8(rd8(ABS(BONUS_RESULT)));
            wr8(ABS(BONUS_RESULT), (u8)A);
        }
    }
    lda8(rd8(ABS(RIDER_BUMPED)));
    if (fz) {
        lda8(rd8(ABSX(TRACK_ROWS + 0x40)));
        wr8(ABS(PLAYER_GROUND), (u8)A);
    }
    lda8(rd8(ABSX(TRACK_ROWS)));
    wr8(ABS(RIDER_TILE_Y), (u8)A);
    cmp8((u8)A, 0x28);
    if (!fc) {
        lda8(rd8(ABS(COURSE_ENDED)));
        if (fz) {
            lda8(0x01);
            wr8(ABS(COURSE_ENDED), 0x01);
            wr8(DP(RACE_END), 0x01);
            wr8(ABS(BONUS_END), 0x01);
            lda8(0x2B);
            push8(0x80); push16(0xD9CE); QueueSoundB_m1x1();
        }
        ret();
        return;
    }
    lda8(rd8(ABSX(TRACK_ROWS + 0x80)));
    if (!fz) {
        lda8(rd8(ABS(0x09C7)));
        ora8(rd8(ABS(RIDER_AIR)));
        if (!fz) {
            lda8(0x45); wr8(ABS(RIDER_TILE_Y), 0x45);
            ret();
            return;
        }
        lda8(rd8(ABSX(TRACK_ROWS)));
        and8(0x40);
        if (!fz) { ret(); return; }
        wr8(ABSX(TRACK_ROWS + 0x80), 0);
        lda8(0x3B);
        wr8(ABSX(TRACK_ROWS + 0x180), 0x3B); wr8(ABSX(TRACK_ROWS + 0x1C0), 0x3B); wr8(ABSX(TRACK_ROWS + 0x200), 0x3B);
        wr8(ABSX(TRACK_ROWS + 0x240), 0x3B); wr8(ABSX(TRACK_ROWS + 0x280), 0x3B); wr8(ABSX(TRACK_ROWS + 0x2C0), 0x3B);
        wr8(ABS(RIDER_TILE), 0x3B);
    }
    lda8(rd8(ABSX(TRACK_ROWS)));
    cmp8((u8)A, 0x84);
    if (fz) {
        lda8(rd8(ABS(0x09C7)));
        if (fz) { lda8(0xFC); wr8(ABS(RIDER_TILE_Y), 0xFC); }
    }
    ret();
}
/* $81:9D71 VelocityDirection (RTL): the 8-way direction ($02DF,X, 0-7)
 * of the velocity $029F/$02AF,X from its signs (table $81:9DE0) and which
 * component dominates. */
void VelocityDirection_m1x1(void)
{
    rep(0x20);
    wr16(DP(0x04), 0);
    wr16(DP(0x06), 0);
    lda16(rd16(ABSX(OBJ_VX)));
    if (fn) {
        eor16(0xFFFF);
        lda16(inc16(A));
        wr16(DP(0x04), inc16(rd16(DP(0x04))));
    }
    wr16(DP(0x00), A);
    lda16(rd16(ABSX(OBJ_VY)));
    if (fn) {
        eor16(0xFFFF);
        lda16(inc16(A));
        wr16(DP(0x06), inc16(rd16(DP(0x06))));
    }
    wr16(DP(0x02), A);
    lda16(rd16(DP(0x04)));
    fc = 0;
    adc16(rd16(DP(0x06)));
    lda16(lsr16(A));
    if (fc) {
        lda16(rd16(DP(0x00)));
        push16(A);
        lda16(rd16(DP(0x02)));
        wr16(DP(0x00), A);
        lda16(pull16());
        wr16(DP(0x02), A);
    }
    lda16(rd16(DP(0x00)));
    cmp16(A, 0x0020);
    if (!fz) {
        for (int i = 0; i < 6; i++) lda16(lsr16(A));
    } else {
        lda16(rd16(DP(0x02)));
        cmp16(A, 0x0020);
        int one = fz;
        if (!one) {
            for (int i = 0; i < 6; i++) lda16(lsr16(A));
            one = fc;
        }
        lda16(one ? 0x0001 : 0x0002);
    }
    wr16(DP(0x08), A);
    lda16(rd16(DP(0x04)));
    lda16(asl16(A));
    ora16(rd16(DP(0x06)));
    Y = (u8)A; nz8((u8)Y);
    lda16(rd16(ABSY(0x9DE0)));
    fc = 0;
    adc16(rd16(DP(0x08)));
    and16(0x0007);
    wr16(ABSX(0x02DF), A);
    sep(0x20);
    retl();
}
/* $82:8525 PackSpriteSizes (RTL): pack the size bytes $1820-$189F (one
 * per sprite, 2 bits used) four to a byte into the OAM high table
 * $1800-$181F (D = $1800 while it works). */
void PackSpriteSizes_m1x1(void)
{
    push16(D);
    lda8(0x18);
    A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
    lda8(0x00);
    D = A; nz16(D);
    ldy8(0x1C);
    do {
        for (int k = 0; k < 4; k++) {
            lda8((u8)Y);
            lda8(asl8((u8)A));
            lda8(asl8((u8)A));
            X = (u8)A; nz8((u8)X);
            u8 o = (u8)(0x20 + 4 * k);
            lda8(rd8(DPX(o + 3)));
            lda8(asl8((u8)A)); lda8(asl8((u8)A)); ora8(rd8(DPX(o + 2)));
            lda8(asl8((u8)A)); lda8(asl8((u8)A)); ora8(rd8(DPX(o + 1)));
            lda8(asl8((u8)A)); lda8(asl8((u8)A)); ora8(rd8(DPX(o)));
            wr8(ABSY(0x1800 + k), (u8)A);
        }
        Y = dec8((u8)Y); Y = dec8((u8)Y); Y = dec8((u8)Y); Y = dec8((u8)Y);
    } while (!fn);
    D = pull16(); nz16(D);
    retl();
}
/* $80:C69B RecordCourseTime (RTL): store the race time $03AE-$03B0 for
 * Stadium course $0928 at $0819 + 3(course - 2); a faster time than the
 * best $083A-$083C becomes the record ($084C = course - 1) and is shown
 * as the race time, else the best is shown (VRAM $18D7, attribute $24).
 * Outside the jump ($84 clear) the race time is cleared. */
void RecordCourseTime_m1x1(void)
{
    push8((u8)X);
    push8((u8)Y);
    lda8(0xFF); wr8(DP(CLOCK_HIDE), 0xFF);
    wr8(DP(0xCC), 0);
    lda8(rd8(ABS(LAP)));
    lda8(dec8((u8)A));
    lda8(dec8((u8)A));
    wr8(DP(0x00), (u8)A);
    lda8(asl8((u8)A));
    fc = 0;
    adc8(rd8(DP(0x00)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABS(RACE_MIN))); wr8(ABSX(0x0819), (u8)A);
    lda8(rd8(ABS(RACE_SEC))); wr8(ABSX(0x081A), (u8)A);
    lda8(rd8(ABS(RACE_FRAMES))); wr8(ABSX(0x081B), (u8)A);
    int record = 0;
    for (int i = 0; i < 3; i++) {
        lda8(rd8(ABS(RECORD_TIME + i)));
        cmp8((u8)A, rd8(ABS(RACE_MIN + i)));
        if (!fc) break;
        if (!fz) { record = 1; break; }
    }
    if (record) {
        lda8(rd8(ABS(RACE_MIN))); wr8(ABS(RECORD_TIME), (u8)A);
        lda8(rd8(ABS(RACE_SEC))); wr8(ABS(RECORD_TIME + 0x01), (u8)A);
        lda8(rd8(ABS(RACE_FRAMES))); wr8(ABS(RECORD_TIME + 0x02), (u8)A);
        lda8(rd8(ABS(LAP)));
        lda8(dec8((u8)A));
        wr8(ABS(RECORD_COURSE), (u8)A);
    }
    lda8(0xD7); wr8(DP(0x00), 0xD7);
    lda8(0x18); wr8(DP(0x01), 0x18);
    lda8(0x24); wr8(DP(0x0F), 0x24);
    if (record) { push16(0xC719); QueueRaceTime_m1x1(); }
    else        { push16(0xC708); QueueBestTime_m1x1(); }
    lda8(rd8(DP(RACE_END)));
    if (fz) {
        wr8(ABS(RACE_MIN), 0);
        wr8(ABS(RACE_SEC), 0);
        wr8(ABS(RACE_FRAMES), 0);
    }
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    retl();
}
/* $81:A1C8 RivalEngineSound (RTL): unless fading, find the rival nearest
 * to the player by distance ($085E words), send its signed distance / 2
 * (APUIO1) and, when it is within $80 of the scroll $9C, its speed / 8
 * (APUIO3, else 0) for the second engine voice. */
void RivalEngineSound_m1x1(void)
{
    lda8(rd8(ABS(FADE_STATE)));
    if (!fz) { retl(); return; }
    ldx8(0x07);
    ldy8(0x0E);
    do {
        rep(0x20);
        lda16(rd16(ABS(RIDER_DIST)));
        wr16(DP(0x00), A);
        lda16(rd16(ABSY(RIDER_DIST)));
        fc = 1;
        sbc16(rd16(DP(0x00)));
        wr16(ABSY(0x0010), A);
        if (fn) { eor16(0xFFFF); lda16(inc16(A)); }
        wr16(ABSY(0x0000), A);
        sep(0x20);
        Y = dec8((u8)Y);
        Y = dec8((u8)Y);
        X = dec8((u8)X);
    } while (!fz);
    rep(0x20);
    lda16(0x000E);
    wr16(DP(0x10), 0x000E);
    ldx8(0x0C);
    lda16(rd16(DP(0x0E)));
    do {
        cmp16(A, rd16(DPX(0x00)));
        if (fc) {
            wr8(DP(0x10), (u8)X);
            lda16(rd16(DPX(0x00)));
        }
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fz);
    sep(0x20);
    X = rd8(DP(0x10)); nz8((u8)X);
    lda8(rd8(DPX(0x00)));
    lda8(lsr8((u8)A));
    Y = rd8(ABSX(0x0011)); nz8((u8)Y);
    if (fn) { eor8(0xFF); lda8(inc8((u8)A)); }
    wr8(ABS(APUIO1), (u8)A);
    rep(0x20);
    lda16(rd16(ABSX(RIDER_DIST)));
    fc = 1;
    sbc16(rd16(ABS(SCROLL_X)));
    if (fn) { eor16(0xFFFF); lda16(inc16(A)); }
    cmp16(A, 0x0080);
    if (fc) {
        lda16(0x0000);
    } else {
        sep(0x20);
        lda8((u8)X);
        lda8(lsr8((u8)A));
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(RIDER_SPEED_HI)));
        A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
        lda8(rd8(ABSX(RIDER_SPEED_LO)));
        rep(0x20);
        lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A));
    }
    sep(0x20);
    wr8(ABS(APUIO3), (u8)A);
    retl();
}
/* $80:B43E QueueTileRows (RTS): queue $11 rows of $10 tile words from the
 * long pointer $02 (consecutive) for VRAM $00, $00 + $20, ...: each a DMA
 * record at the queue end $7F:0000 (mode 1 to $2118, source in bank $7F,
 * $10 * 2 bytes). */
void QueueTileRows_m1x1(void)
{
    push8((u8)X);
    push8((u8)Y);
    ldy8(0x00);
    lda8(rd8(DP(0x10)));
    lda8(asl8((u8)A));
    wr8(DP(0x08), (u8)A);
    wr8(DP(0x09), 0);
    for (;;) {
        rep(0x30);
        lda16(rd16(LONG(VQ_END)));
        X = A; nz16(X);
        sep(0x20);
        lda8(0x80); wr8(LONGX(VQ_VMAIN), 0x80);
        lda8(0x01); wr8(LONGX(VQ_DMAP), 0x01);
        lda8(0x18); wr8(LONGX(VQ_BBAD), 0x18);
        rep(0x20);
        lda16(X);
        fc = 0;
        adc16(0x000C);
        wr16(LONGX(VQ_SRC), A);
        sep(0x20);
        lda8(0x7F); wr8(LONGX(VQ_SRC_BANK), 0x7F);
        lda8(rd8(DP(0x01))); wr8(LONGX(VQ_VMADD_HI), (u8)A);
        lda8(rd8(DP(0x00))); wr8(LONGX(VQ_VMADD), (u8)A);
        lda8(rd8(DP(0x10)));
        wr8(DP(0x06), (u8)A);
        lda8(asl8((u8)A));
        wr8(LONGX(VQ_SIZE), (u8)A);
        lda8(0x00); wr8(LONGX(VQ_SIZE_HI), 0x00);
        do {
            u32 p = rd16(DP(0x02)) | ((u32)rd8(DP(0x04)) << 16);
            lda8(rd8((p + Y) & 0xFFFFFF));
            wr8(LONGX(VQ_DATA), (u8)A);
            Y = inc16(Y);
            lda8(rd8((p + Y) & 0xFFFFFF));
            wr8(LONGX(VQ_DATA_HI), (u8)A);
            X = inc16(X);
            X = inc16(X);
            Y = inc16(Y);
            u8 n = dec8(rd8(DP(0x06)));
            wr8(DP(0x06), n);
        } while (!fz);
        rep(0x20);
        lda16(rd16(LONG(VQ_END)));
        X = A; nz16(X);
        fc = 0; adc16(0x000C);
        fc = 0; adc16(rd16(DP(0x08)));
        wr16(LONG(VQ_END), A);
        wr16(LONGX(VQ_NEXT), A);
        sep(0x20);
        fc = 0;
        lda8(0x20);
        adc8(rd8(DP(0x00)));
        wr8(DP(0x00), (u8)A);
        lda8(0x00);
        adc8(rd8(DP(0x01)));
        wr8(DP(0x01), (u8)A);
        u8 r = dec8(rd8(DP(0x11)));
        wr8(DP(0x11), r);
        if (fz) break;
    }
    sep(0x10);
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    ret();
}
/* $80:AEA0 BrakeRider (RTS): rider X slows down by the step $80:B876,Y
 * (the caller's Y), unless it is already slower than $80:B881 (by crash
 * state), the player is on the jump with $85 set, or (below page 2) it is
 * a rival. */
void BrakeRider_m1x1(void)
{
    wr8(DP(0x00), (u8)Y);
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    if (fz) {
        lda8(rd8(ABSX(RIDER_AIR)));
        lda8(lsr8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSX(RIDER_SPEED_LO)));
        cmp8((u8)A, rd8(ABSY(0xB88A)));
        if (!fc) { ret(); return; }
    }
    Y = rd8(DP(0x00)); nz8((u8)Y);
    lda8(rd8(ABS(RIDER_SCRIPT)));
    cmp8((u8)A, 0x18);
    if (fz) {
        lda8(rd8(DP(SUB_STEP)));
        if (!fz) { ret(); return; }
    }
    lda8(rd8(DP(SCROLL_PAGE)));
    cmp8((u8)A, 0x02);
    if (!fc) {
        cmp8((u8)X, 0x00);
        if (!fz) { ret(); return; }
    }
    lda8(rd8(ABSX(RIDER_SPEED_LO)));
    fc = 1;
    sbc8(rd8(ABSY(0xB87F)));
    wr8(ABSX(RIDER_SPEED_LO), (u8)A);
    if (fc) { ret(); return; }
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    if (!fz) {
        wr8(ABSX(RIDER_SPEED_HI), dec8(rd8(ABSX(RIDER_SPEED_HI))));
        ret();
        return;
    }
    wr8(ABSX(RIDER_SPEED_LO), (u8)A);
    ret();
}
/* $80:B394 QueueTileBlock (RTS): queue a tile block from the list at ($02)
 * (first byte: rows << 4 | words per row, then the tile words) for VRAM
 * $00, one DMA record per row at the queue end $7F:0000, rows $20 words
 * apart. */
void QueueTileBlock_m1x1(void)
{
    push8((u8)X);
    push8((u8)Y);
    ldy8(0x00);
    u32 base = ((u32)DB << 16) + rd16(DP(0x02));
    lda8(rd8((base + Y) & 0xFFFFFF));
    and8(0x0F);
    wr8(DP(0x05), (u8)A);
    lda8(asl8((u8)A));
    wr8(DP(0x08), (u8)A);
    wr8(DP(0x09), 0);
    lda8(rd8((base + Y) & 0xFFFFFF));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(DP(0x04), (u8)A);
    Y = inc8((u8)Y);
    for (;;) {
        rep(0x30);
        lda16(rd16(LONG(VQ_END)));
        X = A; nz16(X);
        sep(0x20);
        lda8(0x80); wr8(LONGX(VQ_VMAIN), 0x80);
        lda8(0x01); wr8(LONGX(VQ_DMAP), 0x01);
        lda8(0x18); wr8(LONGX(VQ_BBAD), 0x18);
        rep(0x20);
        lda16(X);
        fc = 0;
        adc16(0x000C);
        wr16(LONGX(VQ_SRC), A);
        sep(0x20);
        lda8(0x7F); wr8(LONGX(VQ_SRC_BANK), 0x7F);
        lda8(rd8(DP(0x01))); wr8(LONGX(VQ_VMADD_HI), (u8)A);
        lda8(rd8(DP(0x00))); wr8(LONGX(VQ_VMADD), (u8)A);
        lda8(rd8(DP(0x05)));
        wr8(DP(0x06), (u8)A);
        lda8(asl8((u8)A));
        wr8(LONGX(VQ_SIZE), (u8)A);
        lda8(0x00); wr8(LONGX(VQ_SIZE_HI), 0x00);
        do {
            u32 p = ((u32)DB << 16) + rd16(DP(0x02));
            lda8(rd8((p + Y) & 0xFFFFFF));
            wr8(LONGX(VQ_DATA), (u8)A);
            Y = inc16(Y);
            lda8(rd8((p + Y) & 0xFFFFFF));
            wr8(LONGX(VQ_DATA_HI), (u8)A);
            X = inc16(X);
            X = inc16(X);
            Y = inc16(Y);
            u8 n = dec8(rd8(DP(0x06)));
            wr8(DP(0x06), n);
        } while (!fz);
        rep(0x20);
        lda16(rd16(LONG(VQ_END)));
        X = A; nz16(X);
        fc = 0; adc16(0x000C);
        fc = 0; adc16(rd16(DP(0x08)));
        wr16(LONG(VQ_END), A);
        wr16(LONGX(VQ_NEXT), A);
        sep(0x20);
        fc = 0;
        lda8(0x20);
        adc8(rd8(DP(0x00)));
        wr8(DP(0x00), (u8)A);
        lda8(0x00);
        adc8(rd8(DP(0x01)));
        wr8(DP(0x01), (u8)A);
        u8 r = dec8(rd8(DP(0x04)));
        wr8(DP(0x04), r);
        if (fz) break;
    }
    sep(0x10);
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    ret();
}
/* $80:CE9F PlaceRiderShadow (RTS): rider X's shadow sprites at OAM
 * buffer $1600 + $07. On the ground ($019E = 0) near the right edge
 * (column $E1+) they become small/large wrapped sprites; further left, at
 * columns $80+ (blinking by height band), a lane marker sprite is drawn at
 * $1700 + $07 from $80:CF2D. Off the ground they hide, except a rider in
 * the air state 1 keeps them until column $F0. */
void PlaceRiderShadow_m1x1(void)
{
    Y = rd8(DP(0x07)); nz8((u8)Y);
    lda8(rd8(ABSX(RIDER_PAGE)));
    int hide_top = 1;
    if (fz) {
        lda8(rd8(ABSX(RIDER_X)));
        cmp8((u8)A, 0xE1);
        if (fc) {
            cmp8((u8)A, 0xF0);
            if (fc) {
                lda8((u8)Y); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); Y = (u8)A; nz8((u8)Y);
                lda8(0x03);
                wr8(ABSY(OAM_SIZE_BUF), 0x03); wr8(ABSY(OAM_SIZE_BUF + 0x02), 0x03);
                port_ws_rider_sprites(); ret();
                return;
            }
            lda8(0xF0);
            wr8(ABSY(OAM_BUF + 0x01), 0xF0); wr8(ABSY(OAM_BUF + 0x09), 0xF0);
            lda8((u8)Y); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); Y = (u8)A; nz8((u8)Y);
            lda8(0x03);
            wr8(ABSY(OAM_SIZE_BUF + 0x01), 0x03); wr8(ABSY(OAM_SIZE_BUF + 0x03), 0x03);
            port_ws_rider_sprites(); ret();
            return;
        }
        int marker;
        cmp8((u8)A, 0x80);
        if (!fc) marker = 0;
        else {
            cmp8((u8)A, 0xA0);
            if (!fc) marker = 1;
            else {
                cmp8((u8)A, 0xC0);
                if (!fc) { lda8(rd8(DP(FRAME_COUNT))); and8(0x08); marker = !fz; }
                else { lda8(rd8(DP(FRAME_COUNT))); and8(0x04); marker = !fz; }
            }
        }
        if (marker) {
            lda8(0x08); wr8(ABSY(OAM_BUF + 0x100), 0x08);
            lda8(0x88); wr8(ABSY(OAM_BUF + 0x102), 0x88);
            lda8(0x21); wr8(ABSY(OAM_BUF + 0x103), 0x21);
            lda8(rd8(ABSX(RIDER_BAND)));
            push8((u8)Y);
            Y = (u8)A; nz8((u8)Y);
            lda8(rd8(ABSY(0xCF36)));
            Y = pull8(); nz8((u8)Y);
            wr8(ABSY(OAM_BUF + 0x101), (u8)A);
            push8((u8)Y);
            lda8((u8)Y); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); Y = (u8)A; nz8((u8)Y);
            lda8(0x02); wr8(ABSY(OAM_SIZE_BUF + 0x40), 0x02);
            Y = pull8(); nz8((u8)Y);
        }
    } else {
        cmp8((u8)A, 0x02);
        if (!fz) {
            cmp8((u8)A, 0x01);
            if (fz) {
                lda8(rd8(ABSX(RIDER_X)));
                cmp8((u8)A, 0xF0);
                if (!fc) { port_ws_rider_sprites(); ret(); return; }
                hide_top = 0;
            }
        }
    }
    if (hide_top) {
        lda8(0xF0);
        wr8(ABSY(OAM_BUF + 0x01), 0xF0); wr8(ABSY(OAM_BUF + 0x09), 0xF0);
    }
    lda8(0xF0);
    wr8(ABSY(OAM_BUF + 0x05), 0xF0); wr8(ABSY(OAM_BUF + 0x0D), 0xF0);
    port_ws_rider_sprites(); ret();
}
/* $80:E4D4 ShowJumpResult (RTS): freeze the sprites ($1958 + 1), draw the
 * ten score sprites at $1700 (X from $80:E5A6 + 6, tiles $80:E59C), the
 * jump number (count $1578 + 1, one or two digits from $80:E588 and
 * $80:E592) at $1600, and queue the course name tile ($80:E584 by $7C) at
 * VRAM $1886. */
void ShowJumpResult_m1x1(void)
{
    wr8(ABS(SPRITES_FROZEN), inc8(rd8(ABS(SPRITES_FROZEN))));
    ldx8(0x09);
    do {
        lda8((u8)X);
        lda8(asl8((u8)A));
        lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSX(0xE5AF)));
        fc = 0; adc8(0x06);
        wr8(ABSY(OAM_BUF + 0x100), (u8)A);
        lda8(0x50); wr8(ABSY(OAM_BUF + 0x101), 0x50);
        lda8(rd8(ABSX(0xE5A5))); wr8(ABSY(OAM_BUF + 0x102), (u8)A);
        lda8(0x31); wr8(ABSY(OAM_BUF + 0x103), 0x31);
        lda8(0x02); wr8(ABSX(OAM_SIZE_BUF + 0x40), 0x02);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x70); wr8(ABS(OAM_BUF), 0x70);
    lda8(0x80); wr8(ABS(OAM_BUF + 0x04), 0x80);
    lda8(0x38); wr8(ABS(OAM_BUF + 0x01), 0x38); wr8(ABS(OAM_BUF + 0x05), 0x38);
    lda8(0x31); wr8(ABS(OAM_BUF + 0x03), 0x31); wr8(ABS(OAM_BUF + 0x07), 0x31);
    lda8(0x02); wr8(ABS(OAM_SIZE_BUF), 0x02); wr8(ABS(OAM_SIZE_BUF + 0x01), 0x02);
    wr8(ABS(0x1578), inc8(rd8(ABS(0x1578))));
    X = rd8(ABS(0x1578)); nz8((u8)X);
    lda8(rd8(ABSX(0xE591))); wr8(ABS(OAM_BUF + 0x02), (u8)A);
    cmp8((u8)X, 0x0A);
    if (!fc) {
        lda8(rd8(ABSX(0xE59B))); wr8(ABS(OAM_BUF + 0x06), (u8)A);
    } else {
        lda8(rd8(ABS(0xE59B))); wr8(ABS(OAM_BUF + 0x06), (u8)A);
        ldx8(0x00);
        lda8(rd8(ABS(0x1578)));
        for (;;) {
            cmp8((u8)A, 0x0A);
            if (!fc) break;
            X = inc8((u8)X);
            fc = 1;
            sbc8(0x0A);
            WAIT_TICK();
        }
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSY(0xE591))); wr8(ABS(OAM_BUF + 0x02), (u8)A);
        lda8(0x64); wr8(ABS(OAM_BUF + 0x08), 0x64);
        lda8(0x38); wr8(ABS(OAM_BUF + 0x09), 0x38);
        lda8(rd8(ABSX(0xE591))); wr8(ABS(OAM_BUF + 0x0A), (u8)A);
        lda8(0x31); wr8(ABS(OAM_BUF + 0x0B), 0x31);
        lda8(0x02); wr8(ABS(OAM_SIZE_BUF + 0x02), 0x02);
    }
    X = rd8(DP(COURSE_SLOT)); nz8((u8)X);
    lda8(0x11); wr8(ABS(DIGIT_BUF), 0x11);
    lda8(rd8(ABSX(0xE58D))); wr8(ABS(DIGIT_ROW), (u8)A);
    rep(0x20);
    lda16(0x1886); wr16(DP(0x00), 0x1886);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    push16(0xE58B); VramQueueTiles20_m1x1();
    ret();
}
/* $81:9DE4 DrawBigBall (RTS): a 2x2 ball sprite (tiles $DD/$DE/$ED/$EE,
 * attribute $2C) at OAM buffer $17F0 for object $03A3, from its 16-bit
 * position $027F - 10 / $028F + 6; near the left edge (X high byte $FF)
 * or right edge it is clipped by hiding columns and setting the size/X9
 * bits $189C-$189F. */
void DrawBigBall_m1x1(void)
{
    rep(0x20);
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda16(rd16(ABSX(OBJ_X)));
    fc = 0; adc16(0xFFF6);
    wr16(DP(0x00), A);
    lda16(rd16(ABSX(OBJ_Y)));
    fc = 0; adc16(0x0006);
    wr16(DP(0x02), A);
    sep(0x20);
    lda8(0xDD); wr8(ABS(OAM_BUF + 0x1F2), 0xDD);
    lda8(inc8((u8)A)); wr8(ABS(OAM_BUF + 0x1F6), (u8)A);
    lda8(0xED); wr8(ABS(OAM_BUF + 0x1FA), 0xED);
    lda8(inc8((u8)A)); wr8(ABS(OAM_BUF + 0x1FE), (u8)A);
    lda8(0x2C);
    wr8(ABS(OAM_BUF + 0x1F3), 0x2C); wr8(ABS(OAM_BUF + 0x1F7), 0x2C);
    wr8(ABS(OAM_BUF + 0x1FB), 0x2C); wr8(ABS(OAM_BUF + 0x1FF), 0x2C);
    lda8(rd8(DP(0x00)));
    wr8(ABS(OAM_BUF + 0x1F0), (u8)A); wr8(ABS(OAM_BUF + 0x1F8), (u8)A);
    fc = 0; adc8(0x08);
    wr8(ABS(OAM_BUF + 0x1F4), (u8)A); wr8(ABS(OAM_BUF + 0x1FC), (u8)A);
    lda8(rd8(DP(0x02)));
    wr8(ABS(OAM_BUF + 0x1F1), (u8)A); wr8(ABS(OAM_BUF + 0x1F5), (u8)A);
    fc = 0; adc8(0x08);
    wr8(ABS(OAM_BUF + 0x1F9), (u8)A); wr8(ABS(OAM_BUF + 0x1FD), (u8)A);
    lda8(0x02); wr8(DP(0x04), 0x02);
    lda8(rd8(DP(0x01)));
    if (fz) {
        lda8(rd8(DP(0x00)));
        cmp8((u8)A, 0xF0);
        if (fc) {
            lda8(0xF0);
            wr8(ABS(OAM_BUF + 0x1F5), 0xF0); wr8(ABS(OAM_BUF + 0x1FD), 0xF0);
            lda8(rd8(DP(0x04)));
            wr8(ABS(OAM_SIZE_BUF + 0x7C), (u8)A); wr8(ABS(OAM_SIZE_BUF + 0x7E), (u8)A);
            ret();
            return;
        }
        goto all4;
    }
    lda8(inc8((u8)A));
    if (fz) {
        lda8(rd8(DP(0x00)));
        cmp8((u8)A, 0xE9);
        if (fc) {
            wr8(DP(0x04), inc8(rd8(DP(0x04))));
            cmp8((u8)A, 0xF1);
            if (fc) goto all4;
            lda8(0xF0);
            wr8(ABS(OAM_BUF + 0x1F1), 0xF0); wr8(ABS(OAM_BUF + 0x1F9), 0xF0);
            lda8(rd8(DP(0x04)));
            wr8(ABS(OAM_SIZE_BUF + 0x7D), (u8)A); wr8(ABS(OAM_SIZE_BUF + 0x7F), (u8)A);
            ret();
            return;
        }
    }
    lda8(0xF0);
    wr8(ABS(OAM_BUF + 0x1F1), 0xF0); wr8(ABS(OAM_BUF + 0x1F5), 0xF0);
    wr8(ABS(OAM_BUF + 0x1F9), 0xF0); wr8(ABS(OAM_BUF + 0x1FD), 0xF0);
    ret();
    return;
all4:
    lda8(rd8(DP(0x04)));
    wr8(ABS(OAM_SIZE_BUF + 0x7C), (u8)A); wr8(ABS(OAM_SIZE_BUF + 0x7D), (u8)A);
    wr8(ABS(OAM_SIZE_BUF + 0x7E), (u8)A); wr8(ABS(OAM_SIZE_BUF + 0x7F), (u8)A);
    ret();
}
void BonusResultStep_m1x1(void)
{
    u8 t = dec8(rd8(DP(MODE_TIMER)));
    wr8(DP(MODE_TIMER), t);
    if (!fz) { ret(); return; }
    bonus_result_body();
}
/* $81:C3BA DrawJumpCrowd (RTS): the jump-contest flags. At the start
 * ($3C = 1) begin the countdown ($C3 + 1, $8A = $40 unless $92 is set) and
 * set $92 = 1, $3C = 2. Before the run ($92 clear) every 16 frames of $3C
 * ping the APU. Then draw the eight flag poles ($0936 - scroll $9A, Y from
 * $81:C461, tile $E1-$E3 by $3C, attribute $3F) or retire ones that left
 * the screen ($025E = 1); with $54 set, $3C = $B5. */
void DrawJumpCrowd_m1x1(void)
{
    lda8(rd8(DP(0x3C)));
    cmp8((u8)A, 0x01);
    if (fz) {
        lda8(rd8(DP(RACE_STARTED)));
        if (fz) {
            wr8(DP(COUNTDOWN), inc8(rd8(DP(COUNTDOWN))));
            lda8(0x40); wr8(DP(TIMER_A), 0x40);
        }
        lda8(0x01); wr8(DP(RACE_STARTED), 0x01);
        lda8(0x02); wr8(DP(0x3C), 0x02);
    } else {
        Y = rd8(DP(RACE_STARTED)); nz8((u8)Y);
        if (!fz) { ret(); return; }
        Y = rd8(DP(FRAME_COUNT)); nz8((u8)Y);
        and8(0x0F);
        if (fz) { ldy8(0x01); wr8(ABS(APUIO2), 0x01); }
    }
    ldy8(0xE1);
    lda8(rd8(DP(0x3C)));
    cmp8((u8)A, 0x10);
    if (!fc) {
        ldy8(0xE2);
        cmp8((u8)A, 0x08);
        if (!fc) ldy8(0xE3);
    }
    wr8(DP(0x00), (u8)Y);
    ldy8(0x07);
    do {
        lda8(rd8(ABSY(RIDER_RETIRED)));
        if (fz) {
            wr8(DP(0x08), 0);
            lda8(rd8(ABSY(0x0936)));
            fc = 1;
            sbc8(rd8(ABS(BG_SCROLL)));
            wr8(DP(0x01), (u8)A);
            cmp8((u8)A, 0xF0);
            if (fc) {
                cmp8((u8)A, 0xF9);
                if (fc) wr8(DP(0x08), inc8(rd8(DP(0x08))));
                lda8(0x01);
                wr8(ABSY(RIDER_RETIRED), 0x01);
            } else {
                lda8((u8)Y);
                lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
                X = (u8)A; nz8((u8)X);
                lda8(rd8(ABSY(0xC461)));
                wr8(ABSX(OAM_BUF + 0x1A1), (u8)A);
                fc = 0; adc8(0x08);
                wr8(ABSX(OAM_BUF + 0x1A5), (u8)A);
                lda8(rd8(DP(0x00)));
                wr8(ABSX(OAM_BUF + 0x1A2), (u8)A);
                ora8(0x10);
                wr8(ABSX(OAM_BUF + 0x1A6), (u8)A);
                lda8(0x3F);
                wr8(ABSX(OAM_BUF + 0x1A3), 0x3F); wr8(ABSX(OAM_BUF + 0x1A7), 0x3F);
                lda8(rd8(DP(0x01)));
                wr8(ABSX(OAM_BUF + 0x1A0), (u8)A); wr8(ABSX(OAM_BUF + 0x1A4), (u8)A);
                lda8((u8)X);
                lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
                X = (u8)A; nz8((u8)X);
                lda8(rd8(DP(0x08)));
                wr8(ABSX(OAM_SIZE_BUF + 0x68), (u8)A);
            }
        }
        Y = dec8((u8)Y);
    } while (!fn);
    lda8(rd8(DP(0x54)));
    if (!fz) { lda8(0xB5); wr8(DP(0x3C), 0xB5); }
    ret();
}
/* $80:B664 StartBonusResult (RTS): screen setup as in practice mode (BG
 * maps, $1905 = $17, the HDMA scroll channels), the try count $1E95 at
 * VRAM $1957, and when any bonus points were scored ($1E92-$1E94) their
 * total as up to three digits at VRAM $19B3; colour math ($190D $41,
 * COLDATA $E0) and sound 3. */
void StartBonusResult_m1x1(void)
{
    lda8(0x01); wr8(ABS(BG1SC), 0x01); wr8(ABS(0x1900), 0x01);
    lda8(0x09); wr8(ABS(BG2SC), 0x09);
    lda8(0x17); wr8(ABS(SH_TM), 0x17);
    wr8(DP(UPLOAD_SRC), 0);
    wr8(DP(0x87), 0);
    push8(0x80); push16(0xB67D); SetupHdmaChannels_m1x1();
    lda8(0x1A); wr8(ABS(BG3SC), 0x1A);
    lda8(0x44); wr8(ABS(BG12NBA), 0x44);
    lda8(0x01); wr8(ABS(BG34NBA), 0x01);
    lda8(rd8(ABS(0x1E95)));
    wr8(ABS(DIGIT_ROW), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
    lda8(0x21); wr8(ABS(DIGIT_BUF), 0x21);
    rep(0x20);
    lda16(0x1957); wr16(DP(0x00), 0x1957);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    push16(0xB6AD); VramQueueTiles20_m1x1();
    lda8(rd8(ABS(0x1E92)));
    ora8(rd8(ABS(0x1E93)));
    ora8(rd8(ABS(0x1E94)));
    if (!fz) {
        lda8(rd8(ABS(0x1E94)));
        ldy8(0x00);
        for (;;) { cmp8((u8)A, 0x0A); if (!fc) break; sbc8(0x0A); Y = inc8((u8)Y); WAIT_TICK(); }
        wr8(ABS(DIGIT_ROW + 0x02), (u8)A);
        ora8(0x10); wr8(ABS(DIGIT_ROW + 0x06), (u8)A);
        lda8((u8)Y);
        fc = 0; adc8(rd8(ABS(0x1E93)));
        ldy8(0x00);
        for (;;) { cmp8((u8)A, 0x0A); if (!fc) break; sbc8(0x0A); Y = inc8((u8)Y); WAIT_TICK(); }
        wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
        ora8(0x10); wr8(ABS(DIGIT_ROW + 0x05), (u8)A);
        lda8((u8)Y);
        fc = 0; adc8(rd8(ABS(0x1E92)));
        wr8(ABS(DIGIT_ROW), (u8)A);
        ora8(0x10); wr8(ABS(DIGIT_ROW + 0x04), (u8)A);
        lda8(0x00); wr8(ABS(DIGIT_ROW + 0x03), 0x00);
        ora8(0x10); wr8(ABS(DIGIT_ROW + 0x07), (u8)A);
        lda8(rd8(ABS(DIGIT_ROW)));
        if (fz) {
            lda8(0xFF);
            wr8(ABS(DIGIT_ROW), 0xFF); wr8(ABS(DIGIT_ROW + 0x04), 0xFF);
            lda8(rd8(ABS(DIGIT_ROW + 0x01)));
            if (fz) {
                lda8(0xFF);
                wr8(ABS(DIGIT_ROW + 0x01), 0xFF); wr8(ABS(DIGIT_ROW + 0x05), 0xFF);
            }
        }
        lda8(0x24); wr8(ABS(DIGIT_BUF), 0x24);
        rep(0x20);
        lda16(0x19B3); wr16(DP(0x00), 0x19B3);
        lda16(0x03CF); wr16(DP(0x02), 0x03CF);
        sep(0x20);
        push16(0xB72D); VramQueueTiles20_m1x1();
    }
    lda8(0x41); wr8(ABS(SH_CGADSUB), 0x41);
    lda8(0xE0); wr8(ABS(COLDATA), 0xE0);
    lda8(0x03);
    push8(0x80); push16(0xB73D); QueueSoundB_m1x1();
    ret();
}
/* $80:A775 DrawBonusPickup (RTL): when an item was hit ($B7), redraw the
 * two map cells behind it (one or two columns back by the tile under the
 * player, $80:A831 rows and $80:A82C markers) with the tiles from
 * $80:A83B/$80:A840 and $80:A845/$80:A84A, then clear $B7. */
void DrawBonusPickup_m1x1(void)
{
    push8((u8)X);
    lda8(rd8(DP(CLOCK_MINUTE)));
    if (fz) { X = pull8(); nz8((u8)X); retl(); return; }
    wr8(DP(0x10), 0);
    wr8(DP(0x11), 0);
    lda8(rd8(DP(CLOCK_MINUTE)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    rep(0x20);
    lda16(rd16(ABSX(0xA83A)));
    wr16(DP(0x00), A);
    sep(0x20);
    Y = rd8(ABS(RIDER_COLUMN)); nz8((u8)Y);
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x00)) + Y) & 0xFFFFFF));
    Y = rd8(ABS(CLOCK_MINUTE)); nz8((u8)Y);
    cmp8((u8)A, rd8(ABSY(0xA835)));
    if (!fz) {
        cmp8((u8)A, 0x3C);
        if (!fz) cmp8((u8)A, 0x3B);
        if (fz) wr8(DP(0x10), inc8(rd8(DP(0x10))));
    }
    wr8(DP(0x10), inc8(rd8(DP(0x10))));
    queue_bonus_tiles(0xA844, 0xA849);
    push16(0xA7E8); VramQueueTiles_m1x1();
    wr8(DP(0x10), dec8(rd8(DP(0x10))));
    lda8(rd8(DP(CLOCK_MINUTE)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    queue_bonus_tiles(0xA84E, 0xA853);
    push16(0xA830); VramQueueTiles_m1x1();
    wr8(DP(CLOCK_MINUTE), 0);
    X = pull8(); nz8((u8)X);
    retl();
}
/* $80:8F15 ChangeMode (RTS): start the mode in $76 from its first step:
 * APUIO3 = 0, $75 = $77 = 0, fade out ($1923 = -1). */
void ChangeMode_m1x1(void)
{
    wr8(ABS(APUIO3), 0);
    lda8(0x00);
    wr8(DP(MODE_ACTIVE), 0x00);
    wr8(DP(MODE_STEP), 0x00);
    lda8(0xFF);
    wr8(ABS(FADE_STATE), 0xFF);
    ret();
}
/* $80:8F13 NextMode (RTS): $76 + 1, then ChangeMode. */
void NextMode_m1x1(void)
{
    wr8(DP(GAME_MODE), inc8(rd8(DP(GAME_MODE))));
    ChangeMode_m1x1();
}
/* $80:AB62 RaceFinishStep (RTS): after the finish (or a timeout), count
 * $52 down from $10; at 1 the final goes on to the jump contest (mode $13)
 * and a free run to its results (next mode, $86/$87 = 0); a practice run
 * ($0CD7) returns to the title. Before that, clear the race flags
 * ($092D, $0940, $60). In the port a free run raced against the final's
 * rivals still counts as a free run (port_race_is_final). */
void RaceFinishStep_m1x1(void)
{
    lda8(rd8(DP(0x52)));
    if (fz) { lda8(0x10); wr8(DP(0x52), 0x10); }
    cmp8((u8)A, 0x01);
    if (!fz) {
        lda8(0x00);
        wr8(ABS(GAUGE_ON), 0x00);
        wr8(ABS(OVERHEAT_LOCK), 0x00);
        wr8(DP(OVERHEATED), 0x00);
        X = rd8(DP(RACE_END)); nz8((u8)X);
        X = dec8((u8)X);
        if (!fz) { ret(); return; }
        ldx8(0x12);
        lda8(rd8(DP(0x52)));
        lda8(lsr8((u8)A));
        if (!fc) {
            ldy8(0x06);
            lda8(0xFE);
            do {
                X = inc8((u8)X);
                Y = dec8((u8)Y);
            } while (!fn);
        }
        ret();
        return;
    }
    lda8(rd8(DP(FINAL_RACE)));
    if (port_race_is_final()) {
        lda8(0x13);
        wr8(DP(GAME_MODE), 0x13);
        push16(0xAB78); ChangeMode_m1x1();
    } else {
        push16(0xAB7D); NextMode_m1x1();
        wr8(DP(UPLOAD_SRC), 0);
        wr8(DP(0x87), 0);
    }
    lda8(rd8(ABS(ATTRACT_DEMO)));
    if (fz) { ret(); return; }
    ldx8(0x2B);
    do {
        wr8(DPX(0x38), 0);
        X = dec8((u8)X);
    } while (!fn);
    wr8(DP(GAME_MODE), 0);
    wr8(DP(0x79), inc8(rd8(DP(0x79))));
    ChangeMode_m1x1();
}
/* $80:A678 DrawCoinPickup (RTL): when a coin was taken ($B2), redraw the
 * two map cells behind it (one or two columns back, rows $80:A74E, marker
 * $80:A749) with the tiles $80:A758/$80:A75D, count it ($0CDF, CollectCoin),
 * and at the fifth coin start the turbo ($0CE0 = 1, sounds $2D and 1, the
 * meter $0990 = $20). */
void DrawCoinPickup_m1x1(void)
{
    push8((u8)X);
    lda8(rd8(DP(COIN_TAKEN)));
    if (fz) { X = pull8(); nz8((u8)X); retl(); return; }
    lda8(0x01); wr8(DP(0x10), 0x01);
    wr8(DP(0x11), 0);
    lda8(rd8(DP(COIN_TAKEN)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    rep(0x20);
    lda16(rd16(ABSX(0xA757)));
    wr16(DP(0x00), A);
    sep(0x20);
    Y = rd8(ABS(RIDER_COLUMN)); nz8((u8)Y);
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x00)) + Y) & 0xFFFFFF));
    Y = rd8(ABS(COIN_TAKEN)); nz8((u8)Y);
    cmp8((u8)A, rd8(ABSY(0xA752)));
    if (!fz) wr8(DP(0x10), inc8(rd8(DP(0x10))));
    queue_item_tiles(0xB2, 0xA76B, 0xA761, 0xA766);
    push16(0xA6E3); VramQueueTiles_m1x1();
    wr8(DP(0x10), dec8(rd8(DP(0x10))));
    lda8(rd8(DP(COIN_TAKEN)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    queue_item_tiles(0xB2, 0xA76B, 0xA761, 0xA766);
    push16(0xA72B); VramQueueTiles_m1x1();
    wr8(DP(COIN_TAKEN), 0);
    wr8(ABS(COIN_COUNT), inc8(rd8(ABS(COIN_COUNT))));
    push8(0x80); push16(0xA734); CollectCoin_m1x1();
    lda8(rd8(ABS(COIN_COUNT)));
    cmp8((u8)A, 0x05);
    if (fz) {
        lda8(0x2D);
        push8(0x80); push16(0xA741); QueueSoundB_m1x1();
        lda8(0x01); wr8(ABS(TURBO_ON), 0x01);
        push8(0x80); push16(0xA74A); QueueSoundB_m1x1();
        lda8(0x20); wr8(ABS(ENGINE_HEAT), 0x20);
    }
    X = pull8(); nz8((u8)X);
    retl();
}
/* $80:CD3D TurboMeter (RTS): the player's turbo heat $0990 (fraction
 * $098F) rises with A/B held toward the limit $80:BB1B by button (rate
 * $80:BB17) and cools by $0B otherwise; at $20 it overheats ($60, $0940,
 * sounds $2E/$3E), and above $1A on the turbo button the warning sound
 * $2C plays ($AA). Then the meter's four sprites at $17F0 (blinking
 * while overheated, full while the coin turbo $0CE0 runs). */
void TurboMeter_m1x1(void)
{
    u8 y01;
    lda8(rd8(DP(RACE_END)));
    ora8(rd8(ABS(TURBO_ON)));
    if (!fz) { y01 = 0xFF; goto draw; }
    lda8(rd8(ABS(OVERHEAT_LOCK)));
    if (!fz) {
        lda8(rd8(DP(OVERHEATED)));
        lda8(lsr8((u8)A));
        if (fc) { y01 = 0xFF; goto draw; }
        y01 = 0x01;
        goto draw;
    }
    lda8(rd8(ABS(RIDER_INPUT)));
    and8(0xC0);
    if (!fz) {
        Y = rd8(DP(RACE_STARTED)); nz8((u8)Y);
        if (fz) lda8(0x80);
        lda8(asl8((u8)A));
        lda8(rol8((u8)A));
        lda8(rol8((u8)A));
    }
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABS(ENGINE_HEAT)));
    cmp8((u8)A, rd8(ABSY(0xBB24)));
    if (!fc) {
        lda8(rd8(ABS(TURBO_HEAT_FRAC)));
        fc = 0; adc8(rd8(ABSY(0xBB20)));
        wr8(ABS(TURBO_HEAT_FRAC), (u8)A);
        if (fc) wr8(ABS(ENGINE_HEAT), inc8(rd8(ABS(ENGINE_HEAT))));
    } else if (!fz) {
        lda8(rd8(ABS(TURBO_HEAT_FRAC)));
        fc = 1; sbc8(0x0B);
        wr8(ABS(TURBO_HEAT_FRAC), (u8)A);
        if (!fc) {
            lda8(rd8(ABS(ENGINE_HEAT)));
            if (!fz) wr8(ABS(ENGINE_HEAT), dec8(rd8(ABS(ENGINE_HEAT))));
        }
    }
    ldx8(0x00);
    lda8(rd8(ABS(ENGINE_HEAT)));
    cmp8((u8)A, 0x20);
    if (fc) {
        wr8(DP(OVERHEATED), (u8)A);
        wr8(ABS(OVERHEAT_LOCK), (u8)A);
        push8((u8)A);
        lda8(0x2E);
        push8(0x80); push16(0xCDAD); QueueSoundB_m1x1();
        lda8(0x3E);
        push8(0x80); push16(0xCDB3); QueueSoundB_m1x1();
        lda8(pull8());
    }
    cmp8((u8)A, 0x1A);
    if (fc) {
        Y = dec8((u8)Y);
        if (fz) {
            lda8(rd8(DP(TURBO_WARN)));
            if (fz) {
                lda8(rd8(ABS(TURBO_ON)));
                if (fz) {
                    lda8(0x2C);
                    push8(0x80); push16(0xCDCA); QueueSoundB_m1x1();
                }
                wr8(DP(TURBO_WARN), (u8)A);
            }
            y01 = 0xFF;
            goto draw;
        }
    }
    lda8(rd8(DP(TURBO_WARN)));
    if (!fz) {
        lda8(0x2D);
        push8(0x80); push16(0xCDD8); QueueSoundB_m1x1();
        wr8(DP(TURBO_WARN), 0);
    }
    y01 = 0xFF;
draw:
    lda8(y01);
    wr8(DP(0x01), y01);
    lda8(rd8(ABS(ENGINE_HEAT)));
    fc = 0; adc8(0x58);
    X = rd8(ABS(TURBO_ON)); nz8((u8)X);
    if (!fz) lda8(0x78);
    wr8(DP(0x00), (u8)A);
    ldx8(0x0C);
    lda8(rd8(ABS(TURBO_ON)));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    do {
        lda8(rd8(DP(0x00)));
        wr8(ABSX(OAM_BUF + 0x1F0), (u8)A);
        fc = 0; adc8(0x08);
        wr8(DP(0x00), (u8)A);
        cmp8((u8)A, 0x70);
        lda8(fc ? 0xC7 : 0xF0);
        wr8(ABSX(OAM_BUF + 0x1F1), (u8)A);
        lda8(rd8(ABSY(0xCE20)));
        wr8(ABSX(OAM_BUF + 0x1F2), (u8)A);
        lda8(0x2F);
        wr8(ABSX(OAM_BUF + 0x1F3), 0x2F);
        Y = inc8((u8)Y);
        X = dec8((u8)X); X = dec8((u8)X); X = dec8((u8)X); X = dec8((u8)X);
    } while (!fn);
    ret();
}
/* $80:9B1B EnterNumberStep (RTS): a three-digit entry ($1570-$1572,
 * cursor $044F): up/down ($21 bits 3/2, sound $33) step the digit 0-9
 * with wrap, left/right move the cursor (wrapping 0-2); the number never
 * becomes 000 (the last digit is forced to 1, or 9 when stepping down).
 * Queue the digits ($156F, size $23, lower row +$10) at VRAM $0B0F and the
 * cursor arrows; Start ($21 bit 4) confirms ($0450 + 1) and queues the
 * next screen's tiles ($7E:5A80, $20 x 2 rows at VRAM $0300). */
void EnterNumberStep_m1x1(void)
{
    X = rd8(ABS(ENTRY_CURSOR)); nz8((u8)X);
    ldy8(0x01);
    lda8(rd8(DP(P1_PRESS_HI)));
    and8(0x0C);
    if (!fz) {
        lda8(0x33);
        push8(0x80); push16(0x9B2B); QueueSoundB_m1x1();
        lda8(rd8(DP(P1_PRESS_HI)));
        and8(0x08);
        if (!fz) {
            lda8(rd8(ABSX(ENTRY_DIGITS)));
            lda8(inc8((u8)A));
            wr8(ABSX(ENTRY_DIGITS), (u8)A);
            cmp8((u8)A, 0x0A);
            if (fc) wr8(ABSX(ENTRY_DIGITS), 0);
        } else {
            ldy8(0x09);
            lda8(rd8(ABSX(ENTRY_DIGITS)));
            lda8(dec8((u8)A));
            wr8(ABSX(ENTRY_DIGITS), (u8)A);
            if (fn) { lda8(0x09); wr8(ABSX(ENTRY_DIGITS), 0x09); }
        }
    }
    lda8(rd8(DP(P1_PRESS_HI)));
    and8(0x03);
    if (!fz) {
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(0x9C1E)));
        fc = 0; adc8(rd8(ABS(ENTRY_CURSOR)));
        if (fn) lda8(0x02);
        else { cmp8((u8)A, 0x03); if (fc) lda8(0x00); }
        wr8(ABS(ENTRY_CURSOR), (u8)A);
    }
    lda8(rd8(ABS(ENTRY_DIGITS)));
    ora8(rd8(ABS(ENTRY_DIGITS + 0x01)));
    ora8(rd8(ABS(ENTRY_DIGITS + 0x02)));
    if (fz) {
        cmp8((u8)X, 0x02);
        if (!fz) ldy8(0x01);
        wr8(ABS(ENTRY_DIGITS + 0x02), (u8)Y);
    }
    for (int i = 0; i < 3; i++) {
        lda8(rd8(ABS(ENTRY_DIGITS + i)));
        ora8(0x10);
        wr8(ABS(0x1573 + i), (u8)A);
    }
    rep(0x20);
    lda16(0x0B0F); wr16(DP(0x00), 0x0B0F);
    lda16(0x156F); wr16(DP(0x02), 0x156F);
    sep(0x20);
    lda8(0x23); wr8(ABS(0x156F), 0x23);
    lda8(0x01); wr8(DP(0x0F), 0x01);
    push16(0x9BB4); VramQueueTiles_m1x1();
    lda8(rd8(ABS(ENTRY_CURSOR)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    fc = 0; adc8(0x78);
    wr8(ABS(OAM_BUF), (u8)A); wr8(ABS(OAM_BUF + 0x04), (u8)A);
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x18);
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(DP(0x00), (u8)A);
    lda8(0x8E);
    fc = 0; adc8(rd8(DP(0x00)));
    wr8(ABS(OAM_BUF + 0x01), (u8)A);
    lda8(0x78);
    fc = 1; sbc8(rd8(DP(0x00)));
    wr8(ABS(OAM_BUF + 0x05), (u8)A);
    lda8(0xFE); wr8(ABS(OAM_BUF + 0x02), 0xFE); wr8(ABS(OAM_BUF + 0x06), 0xFE);
    lda8(0x21); wr8(ABS(OAM_BUF + 0x03), 0x21);
    lda8(0xA1); wr8(ABS(OAM_BUF + 0x07), 0xA1);
    wr8(ABS(OAM_SIZE_BUF), 0);
    wr8(ABS(OAM_SIZE_BUF + 0x01), 0);
    lda8(rd8(DP(P1_PRESS_HI)));
    and8(0x10);
    if (!fz) {
        wr8(ABS(END_STEP), inc8(rd8(ABS(END_STEP))));
        lda8(0x80); wr8(DP(0x02), 0x80);
        lda8(0x5A); wr8(DP(0x03), 0x5A);
        lda8(0x7E); wr8(DP(0x04), 0x7E);
        lda8(0x20); wr8(DP(0x10), 0x20);
        lda8(0x02); wr8(DP(0x11), 0x02);
        lda8(0x00); wr8(DP(0x00), 0x00);
        lda8(0x03); wr8(DP(0x01), 0x03);
        push16(0x9C1C); QueueTileRows_m1x1();
    }
    ret();
}
/* $80:BE6F RivalTarget (RTS, Y = the rider's lane state $019E): CPU rider
 * X's target speed $0236/$022E,X. After the race (\$84) high byte 3; riders
 * flagged in $80:BEB9 (1-3) ride at the player's speed while the player is
 * on the ground and moving, keeping near it (+1 up to 4 when behind in the
 * same lane); otherwise the high byte from $80:BAEF by lane state and a
 * random low byte from $80:BAE7 ($2F,X & $0F). */
void RivalTarget_m1x1(void)
{
    lda8(rd8(DP(RACE_END)));
    if (!fz) {
        lda8(0x03);
        goto table_lo;
    }
    lda8(rd8(ABSX(0xBEC2)));
    if (!fz) {
        lda8(rd8(ABSX(RIDER_PAGE)));
        lda8(lsr8((u8)A));
        lda8(lsr8((u8)A));
        if (!fc) {
            lda8(rd8(ABS(RIDER_SPEED_HI)));
            if (!fz) {
                wr8(ABSX(RIDER_TARGET_HI), (u8)A);
                lda8(rd8(ABS(RIDER_SPEED_LO)));
                wr8(ABSX(RIDER_TARGET_LO), (u8)A);
                lda8(rd8(ABSX(RIDER_PAGE)));
                int bump = fz;
                if (!bump) {
                    lda8(rd8(ABSX(RIDER_X)));
                    cmp8((u8)A, 0x40);
                    if (fc) goto table;
                    lda8(rd8(ABS(RIDER_BAND)));
                    cmp8((u8)A, rd8(ABS(RIDER_BAND + 0x03)));
                    bump = !fz;
                }
                if (bump) {
                    lda8(rd8(ABSX(RIDER_TARGET_HI)));
                    cmp8((u8)A, 0x04);
                    if (!fc) wr8(ABSX(RIDER_TARGET_HI), inc8(rd8(ABSX(RIDER_TARGET_HI))));
                }
                pace_target();
                ret();
                return;
            }
        }
    }
table:
    lda8(rd8(ABSY(0xBAF8)));
table_lo:
    wr8(ABSX(RIDER_TARGET_HI), (u8)A);
    lda8(rd8(DPX(0x2F)));
    and8(0x0F);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xBAF0)));
    wr8(ABSX(RIDER_TARGET_LO), (u8)A);
    pace_target();
    ret();
}
/* $80:CC21 DrawRiderShadow (RTS): rider X's shadow while it is in the air
 * on a ramp ($08BC): two sprites under it (DrawRiderShadowPair at OAM
 * buffer $16C0 + Y), blinking every 4 frames, palette from the track piece
 * (or $0E off the track and above height $58), attribute | $21, the lower
 * one hidden while the speed high byte is zero. Only for a moving rider on
 * the original screen ($019E = 1) from column $10. */
void DrawRiderShadow_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_ON_RAMP)));
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    ora8(rd8(ABSX(RIDER_SPEED_LO)));
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_HEIGHT)));
    if (!fz) { ret(); return; }
    int margin = port_ws_shadow_margin();
    if (margin) {
        lda8(rd8(ABSX(RIDER_X)));
    } else {
        lda8(rd8(ABSX(RIDER_PAGE)));
        cmp8((u8)A, 0x01);
        if (!fz) { ret(); return; }
        lda8(rd8(ABSX(RIDER_X)));
        cmp8((u8)A, 0x10);
        if (!fc) { ret(); return; }
    }
    push16(0xCC43); DrawRiderShadowPair_m1x1();
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x04);
    if (!fz) {
        lda8(0xF0); wr8(ABSY(OAM_BUF + 0xC2), 0xF0);
        lda8(0xE0); wr8(ABSY(OAM_BUF + 0xC6), 0xE0);
    }
    lda8(rd8(ABSX(RIDER_BAND)));
    if (fz) {
        lda8(0x0E); wr8(DP(0x02), 0x0E);
    } else {
        push8((u8)Y);
        Y = rd8(ABSX(RIDER_TILE_Y)); nz8((u8)Y);
        lda8(rd8(ABSY(0xAD3C)));
        Y = pull8(); nz8((u8)Y);
        wr8(DP(0x02), (u8)A);
        cmp8((u8)A, 0x00);
        if (fz) {
            lda8(0xF0);
            wr8(ABSY(OAM_BUF + 0xC1), 0xF0);
            wr8(ABSY(OAM_BUF + 0xC5), 0xF0);
        }
    }
    lda8(rd8(ABSX(RIDER_Y)));
    cmp8((u8)A, 0x58);
    if (!fc) lda8(0x0E);
    else lda8(rd8(DP(0x02)));
    ora8(0x21);
    wr8(ABSY(OAM_BUF + 0xC3), (u8)A);
    wr8(ABSY(OAM_BUF + 0xC7), (u8)A);
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    if (fz) { lda8(0xF8); wr8(ABSY(OAM_BUF + 0xC5), 0xF8); }
    if (margin) port_ws_shadow_fix();
    ret();
}
/* $80:9FEA BsxEndIfFlagged (RTS): read the broadcast header; when its
 * flags ($01 & 3) are 3, hand control back to the receiver ($2197 = $80). */
void BsxEndIfFlagged_m1x1(void)
{
    push8(0x80); push16(0x9FED); BsxReadHeader3_m1x1();
    lda8(rd8(DP(0x01)));
    and8(0x03);
    cmp8((u8)A, 0x03);
    if (fz) { lda8(0x80); wr8(ABS(0x2197), 0x80); }
    ret();
}
/* $80:EE6F CopyPieceColumn (RTS): copy the words at ($05) + Y + 2, ... into
 * the VRAM queue $7F:000E + X until X = $1E. */
void CopyPieceColumn_m1x1(void)
{
    do {
        WAIT_TICK();
        Y = inc8((u8)Y); Y = inc8((u8)Y);
        rep(0x20);
        lda16(rd16((((u32)DB << 16) + rd16(DP(0x05)) + Y) & 0xFFFFFF));
        wr16(LONGX(VQ0_DATA), A);
        X = inc8((u8)X); X = inc8((u8)X);
        cmp8((u8)X, 0x1E);
    } while (!fz);
    sep(0x20);
    ret();
}
/* $80:DBE1 CopyPieceColumnPal2 (RTS): as CopyPieceColumn, each word masked
 * to $20FF and given palette 2 ($0800). */
void CopyPieceColumnPal2_m1x1(void)
{
    do {
        WAIT_TICK();
        Y = inc8((u8)Y); Y = inc8((u8)Y);
        rep(0x20);
        lda16(rd16((((u32)DB << 16) + rd16(DP(0x05)) + Y) & 0xFFFFFF));
        and16(0x20FF);
        ora16(0x0800);
        wr16(LONGX(VQ0_DATA), A);
        X = inc8((u8)X); X = inc8((u8)X);
        cmp8((u8)X, 0x1E);
    } while (!fz);
    sep(0x20);
    ret();
}
/* $80:B0D9 RetargetRival (RTS): rival X (not rider 7) picks a new target
 * speed (RivalTarget) every time it passes a 32-column boundary with the
 * random $2E,X at $C0 or more (Y = 1 when above $C0); rider 7 always. */
void RetargetRival_m1x1(void)
{
    ldy8(0x00);
    cmp8((u8)X, 0x07);
    if (!fz) {
        lda8(rd8(ABSX(RIDER_X)));
        and8(0x10);
        if (fz) { ret(); return; }
        lda8(rd8(DPX(0x2E)));
        cmp8((u8)A, 0xC0);
        if (!fc) { ret(); return; }
        if (!fz) Y = inc8((u8)Y);
    }
    push16(0xB0F1); RivalTarget_m1x1();
    ret();
}
/* $80:DA5E BonusTimerStep (RTS): the bonus course's countdown: every $3C
 * frames ($B5) one second off $B4; at zero the run ends ($84 = 2, $1990 = 2
 * unless set, sound $2D). Then the counter display. */
void BonusTimerStep_m1x1(void)
{
    u8 t = dec8(rd8(DP(BONUS_TICK)));
    wr8(DP(BONUS_TICK), t);
    if (fz) {
        lda8(0x3C); wr8(DP(BONUS_TICK), 0x3C);
        u8 s = dec8(rd8(DP(BONUS_SECONDS)));
        wr8(DP(BONUS_SECONDS), s);
        if (fz) {
            lda8(0x02); wr8(DP(RACE_END), 0x02);
            X = rd8(ABS(BONUS_END)); nz8((u8)X);
            if (fz) wr8(ABS(BONUS_END), (u8)A);
            lda8(0x2D);
            push8(0x80); push16(0xDA7B); QueueSoundB_m1x1();
        }
    }
    push16(0xDA7E); QueueBonusCount_m1x1();
    ret();
}
/* $80:DA33 BonusScrollStep (RTS): while the bonus run lasts the track
 * scrolls with the player (ScrollByPlayerSpeed); after it the player rides
 * on across the screen ($0196 + speed) and at column $F0 leaves it ($84 =
 * $019E = 2, sound $2D, the counter display); otherwise the speed is
 * cleared for the frame. */
void BonusScrollStep_m1x1(void)
{
    lda8(rd8(DP(RACE_END)));
    if (fz) { ScrollByPlayerSpeed_m1x1(); return; }
    lda8(rd8(ABS(RIDER_STEP)));
    fc = 0; adc8(rd8(ABS(RIDER_X)));
    wr8(ABS(RIDER_X), (u8)A);
    cmp8((u8)A, 0xF0);
    if (fc) {
        lda8(0x02);
        wr8(DP(RACE_END), 0x02);
        wr8(ABS(RIDER_PAGE), 0x02);
        lda8(0x2D);
        push8(0x80); push16(0xDA54); QueueSoundB_m1x1();
        push16(0xDA7E); QueueBonusCount_m1x1();
        ret();
        return;
    }
    lda8(0x00);
    wr8(ABS(RIDER_STEP), 0x00);
    ret();
}
/* $80:9D1C LoadEntryScreenStep (RTS): count $88 down; at zero queue the
 * entry screen's tiles ($7E:5B00, $20 x 2 rows at VRAM $0300) and go to
 * step 9 ($0450). */
void LoadEntryScreenStep_m1x1(void)
{
    u8 t = dec8(rd8(DP(MODE_TIMER)));
    wr8(DP(MODE_TIMER), t);
    if (!fz) { ret(); return; }
    lda8(0x00); wr8(DP(0x02), 0x00);
    lda8(0x5B); wr8(DP(0x03), 0x5B);
    lda8(0x7E); wr8(DP(0x04), 0x7E);
    lda8(0x20); wr8(DP(0x10), 0x20);
    lda8(0x02); wr8(DP(0x11), 0x02);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x03); wr8(DP(0x01), 0x03);
    push16(0x9D3E); QueueTileRows_m1x1();
    lda8(0x09); wr8(ABS(END_STEP), 0x09);
    ret();
}
/* $81:98FF FadeObjectStep (RTS): object $03A3's palette fade: every $C0
 * frames ($88) move the fade start $B9 by 8 until $16, then advance the
 * object's state ($030F,X); between steps FadePalette. */
void FadeObjectStep_m1x1(void)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(DP(MODE_TIMER)));
    if (fz) {
        lda8(rd8(DP(0xB9)));
        cmp8((u8)A, 0x16);
        if (fc) {
            wr8(ABSX(OBJ_STATE), inc8(rd8(ABSX(OBJ_STATE))));
            X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
            ret();
            return;
        }
        fc = 0; adc8(0x08);
        wr8(DP(0xB9), (u8)A);
        lda8(0xC0); wr8(DP(MODE_TIMER), 0xC0);
    }
    wr8(DP(MODE_TIMER), dec8(rd8(DP(MODE_TIMER))));
    push8(0x81); push16(0x9921); FadePalette_m1x1();
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    ret();
}
/* $80:C596 TickRaceClock (RTS): the race clock $03AE:$03AF:$03B0 (minutes,
 * seconds, frames): carry the frames at $3C into the seconds and the
 * seconds at $3C into the minutes; unless $CB, show it (QueueRaceTime with
 * attribute $20, which returns for this routine). */
void TickRaceClock_m1x1(void)
{
    lda8(0x20); wr8(DP(0x0F), 0x20);
    lda8(rd8(ABS(RACE_FRAMES)));
    cmp8((u8)A, 0x3C);
    if (fc) {
        wr8(ABS(RACE_SEC), inc8(rd8(ABS(RACE_SEC))));
        lda8(rd8(ABS(RACE_SEC)));
        cmp8((u8)A, 0x3C);
        if (fc) {
            wr8(ABS(RACE_MIN), inc8(rd8(ABS(RACE_MIN))));
            wr8(ABS(RACE_SEC), 0);
        }
        wr8(ABS(RACE_FRAMES), 0);
    }
    lda8(rd8(DP(CLOCK_HIDE)));
    if (fz) { QueueRaceTime_m1x1(); return; }
    ret();
}
/* $80:E95F ShowJumpCount (RTS): the jump count $1569 as five digits plus
 * the score label (BuildJumpScore), queued at VRAM $1970, and the label
 * tiles $80:E989 at VRAM $1967. */
void ShowJumpCount_m1x1(void)
{
    rep(0x20);
    lda16(rd16(ABS(JUMP_COUNT)));
    push16(0xE966); BuildScoreDigits_m0x1();
    push8(0x80); push16(0xE96A); BuildJumpScore_m1x1();
    rep(0x20);
    lda16(0x1970); wr16(DP(0x00), 0x1970);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xE97F); VramQueueTiles_m1x1();
    rep(0x20);
    lda16(0x1967); wr16(DP(0x00), 0x1967);
    lda16(0xE992); wr16(DP(0x02), 0xE992);
    sep(0x20);
    push16(0xE990); VramQueueTiles20_m1x1();
    ret();
}
/* $80:E9BB ShowBonusPoints (RTS): the points for jump rank $156B (table
 * $80:F155) as five digits at VRAM $19B3, and the label tiles $80:E9E7 at
 * VRAM $19A7. */
void ShowBonusPoints_m1x1(void)
{
    rep(0x20);
    Y = rd8(ABS(JUMP_RANK)); nz8((u8)Y);
    lda16(rd16(ABSY(0xF122)));
    and16(0x00FF);
    push16(0xE9C8); BuildScoreDigits_m0x1();
    rep(0x20);
    lda16(0x19B3); wr16(DP(0x00), 0x19B3);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xE9DD); VramQueueTiles_m1x1();
    rep(0x20);
    lda16(0x19A7); wr16(DP(0x00), 0x19A7);
    lda16(0xE9F0); wr16(DP(0x02), 0xE9F0);
    sep(0x20);
    push16(0xE9EE); VramQueueTiles20_m1x1();
    ret();
}
/* $81:8C3C CopyTrackColumn (RTL): the track streamer's column: from the
 * piece words at ($05) + Y + 2, ... each tile (& $07FF | $0800, palette 2)
 * into the VRAM queue $7F:000C + X and its height bits (high byte >> 3)
 * into $7F:200C + X, until X = $1E. */
void CopyTrackColumn_m1x1(void)
{
    do {
        Y = inc8((u8)Y); Y = inc8((u8)Y);
        rep(0x20);
        lda16(rd16((((u32)DB << 16) + rd16(DP(0x05)) + Y) & 0xFFFFFF));
        push16(A);
        and16(0x07FF);
        ora16(0x0800);
        wr16(LONGX(VQ_DATA), A);
        lda16(pull16());
        A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
        and16(0x00FF);
        lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A));
        wr16(LONGX(0x7F200C), A);
        X = inc8((u8)X); X = inc8((u8)X);
        cmp8((u8)X, 0x1E);
    } while (!fz);
    sep(0x20);
    retl();
}
/* $80:CE66 PlaceAllRiderShadows (RTS): PlaceRiderShadow for every rider
 * slot 7..0 in use ($0950), OAM offset from $80:BAC7; a rider on screen in
 * columns $70-$8F sets $092A. */
void PlaceAllRiderShadows_m1x1(void)
{
    ldy8(0x07);
    do {
        wr8(DP(0x05), (u8)Y);
        lda8(rd8(ABSY(RANK_ORDER)));
        if (!fz) {
            X = (u8)A; nz8((u8)X);
            lda8(rd8(ABSY(0xBAD0)));
            wr8(DP(0x07), (u8)A);
            lda8(rd8(ABSX(RIDER_PAGE)));
            cmp8((u8)A, 0x01);
            if (fz) {
                lda8(rd8(ABSX(RIDER_X)));
                cmp8((u8)A, 0x70);
                if (fc) {
                    cmp8((u8)A, 0x90);
                    if (!fc) { lda8(0x01); wr8(ABS(0x092A), 0x01); }
                }
            }
            push16(0xCE8E); PlaceRiderShadow_m1x1();
        }
        Y = rd8(DP(0x05)); nz8((u8)Y);
        Y = dec8((u8)Y);
    } while (!fn);
    ret();
}
/* $81:98B6 SkyFadeStep (RTS): fade the sky (FadeSkyColours); when the
 * object $03A3's timer $031F runs out, advance its state, set colours
 * $1C02-$1C16 to $6D69, flag the upload, $A5 + 1, $88 = $C0, $B9 = 6. */
void SkyFadeStep_m1x1(void)
{
    push8(0x81); push16(0x98B9); FadeSkyColours_m1x1();
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    rep(0x20);
    u16 t = dec16(rd16(ABSX(OBJ_STEP)));
    wr16(ABSX(OBJ_STEP), t);
    if (fz) {
        wr16(ABSX(OBJ_STATE), inc16(rd16(ABSX(OBJ_STATE))));
        rep(0x20);
        lda16(0x6D69);
        for (u16 a = 0x1C02; a <= 0x1C16; a += 2) wr16(ABS(a), 0x6D69);
        sep(0x20);
        wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
        wr8(DP(0xA5), inc8(rd8(DP(0xA5))));
        lda8(0xC0); wr8(DP(MODE_TIMER), 0xC0);
        lda8(0x06); wr8(DP(0xB9), 0x06);
    }
    sep(0x20);
    ret();
}
/* $80:9C22 EntryConfirmStep (RTS): on B ($21 or $26 bit 7): with a press
 * on the second pad ($26) leave for mode $16 (fade, $0450 + 1, sound $45);
 * otherwise redraw the entry screen ($7E:5A00, $20 x 2 rows at VRAM $0300)
 * and go back a step ($0450 - 1). */
void EntryConfirmStep_m1x1(void)
{
    lda8(rd8(DP(P1_PRESS_LO)));
    ora8(rd8(DP(P1_PRESS_HI)));
    and8(0x80);
    if (fz) { ret(); return; }
    lda8(rd8(DP(P1_PRESS_LO)));
    if (!fz) {
        wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
        wr8(ABS(END_STEP), inc8(rd8(ABS(END_STEP))));
        lda8(0x45);
        push8(0x80); push16(0x9C39); QueueSoundB_m1x1();
        lda8(0x16); wr8(DP(GAME_MODE), 0x16);
        wr8(DP(MODE_ACTIVE), 0);
        wr8(DP(MODE_STEP), 0);
        ret();
        return;
    }
    lda8(0x00); wr8(DP(0x02), 0x00);
    lda8(0x5A); wr8(DP(0x03), 0x5A);
    lda8(0x7E); wr8(DP(0x04), 0x7E);
    lda8(0x20); wr8(DP(0x10), 0x20);
    lda8(0x02); wr8(DP(0x11), 0x02);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x03); wr8(DP(0x01), 0x03);
    push16(0x9C61); QueueTileRows_m1x1();
    wr8(ABS(END_STEP), dec8(rd8(ABS(END_STEP))));
    ret();
}
/* $80:9ACD EntryIntroStep (RTS): zoom $A2 in by 2 a frame; at $0140 sound
 * $2D, next step ($0450 + 1), queue the entry screen ($7E:5A00, $20 x 2
 * rows at VRAM $0300) and reset the number to 001 ($1570-$1572, $156F,
 * cursor $044F). */
void EntryIntroStep_m1x1(void)
{
    rep(0x20);
    wr16(ABS(ZOOM), inc16(rd16(ABS(ZOOM))));
    wr16(ABS(ZOOM), inc16(rd16(ABS(ZOOM))));
    lda16(rd16(ABS(ZOOM)));
    cmp16(A, 0x0140);
    if (fc) {
        sep(0x20);
        lda8(0x2D);
        push8(0x80); push16(0x9AE4); QueueSoundB_m1x1();
        wr8(ABS(END_STEP), inc8(rd8(ABS(END_STEP))));
        lda8(0x00); wr8(DP(0x02), 0x00);
        lda8(0x5A); wr8(DP(0x03), 0x5A);
        lda8(0x7E); wr8(DP(0x04), 0x7E);
        lda8(0x20); wr8(DP(0x10), 0x20);
        lda8(0x02); wr8(DP(0x11), 0x02);
        lda8(0x00); wr8(DP(0x00), 0x00);
        lda8(0x03); wr8(DP(0x01), 0x03);
        push16(0x9B06); QueueTileRows_m1x1();
        lda8(0x01); wr8(ABS(ENTRY_DIGITS + 0x02), 0x01);
        wr8(ABS(ENTRY_DIGITS + 0x01), 0);
        wr8(ABS(ENTRY_DIGITS), 0);
        wr8(ABS(0x156F), 0);
        wr8(ABS(ENTRY_CURSOR), 0);
    }
    sep(0x20);
    ret();
}
/* $80:FB7B OffTrackBrake (RTS, Y kept): rider X on rough ground (not
 * crashed, $020E 0 or bit 1 set) brakes by step 6 (5 with the turbo
 * button, $017E bit 6) and counts as on a ramp ($08BC = 1); on tile type 1
 * the player's turbo heat fraction $098F is held at $1200 at most. */
void OffTrackBrake_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_AIR)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_JUMP)));
    if (!fz) {
        lda8(lsr8((u8)A));
        lda8(lsr8((u8)A));
        if (!fc) { ret(); return; }
    }
    wr8(DP(0x01), (u8)Y);
    ldy8(0x04);
    lda8(rd8(ABSX(RIDER_INPUT)));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    if (fc) Y = dec8((u8)Y);
    lda8(0x01); wr8(ABSX(RIDER_ON_RAMP), 0x01);
    Y = inc8((u8)Y); Y = inc8((u8)Y);
    push16(0xFB9E); BrakeRider_m1x1();
    Y = rd8(ABSX(RIDER_TILE_Y)); nz8((u8)Y);
    lda8(rd8(ABSY(0xAD3C)));
    cmp8((u8)A, 0x01);
    if (fz) {
        cmp8((u8)X, 0x00);
        if (fz) {
            rep(0x20);
            lda16(rd16(ABS(TURBO_HEAT_FRAC)));
            cmp16(A, 0x1200);
            if (fc) { lda16(0x1200); wr16(ABS(TURBO_HEAT_FRAC), 0x1200); }
        }
        sep(0x20);
    }
    Y = rd8(DP(0x01)); nz8((u8)Y);
    ret();
}
/* $80:F064 BonusSignStep (RTS): while the bonus sign is up ($97) draw it
 * eleven columns ahead of the track ($0279 + $0B, tilemap $0B20 or the other
 * screen): the sign block $80:F0F1 on the first frame (remembering the
 * address in $98), then the blank block $80:F0FC once the column moved on. */
void BonusSignStep_m1x1(void)
{
    lda8(rd8(DP(BONUS_SIGN)));
    if (fz) { sep(0x20); ret(); return; }
    lda8(0x28); wr8(DP(0x0F), 0x28);
    rep(0x20);
    lda16(rd16(ABS(0x0279)));
    fc = 0; adc16(0x000B);
    and16(0x0020);
    eor16(0x0020);
    A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
    lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A));
    fc = 0; adc16(0x0B20);
    wr16(DP(0x00), A);
    lda16(rd16(ABS(0x0279)));
    fc = 0; adc16(0x000B);
    and16(0x001F);
    ora16(rd16(DP(0x00)));
    wr16(DP(0x00), A);
    sep(0x20);
    Y = rd8(DP(BONUS_SIGN)); nz8((u8)Y);
    Y = dec8((u8)Y);
    if (fz) {
        rep(0x20);
        wr16(DP(0x98), A);
        lda16(0xF0BE); wr16(DP(0x02), 0xF0BE);
        sep(0x20);
        wr8(DP(BONUS_SIGN), inc8(rd8(DP(BONUS_SIGN))));
    } else {
        rep(0x20);
        cmp16(A, rd16(DP(0x98)));
        if (fz) { sep(0x20); ret(); return; }
        wr16(DP(0x98), 0);
        lda16(0xF0C9); wr16(DP(0x02), 0xF0C9);
        sep(0x20);
    }
    push16(0xF0BA); QueueTileBlock_m1x1();
    sep(0x20);
    ret();
}
/* $80:CAEE DrawRider (RTS): rider X's 2x2 sprite at column $0196, height
 * $01B6 - 8 (the player outside races and screen $0C rides at $28 when low
 * or flagged $0904; a flagged rival off the bottom is not drawn), with the
 * pose from RiderPose: tiles $80:BA99, the slot's animation frame $1961
 * from $80:CB58 (frame $14 for a landed player at the end of the course). */
void DrawRider_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_X)));
    wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSX(RIDER_Y)));
    fc = 1; sbc8(0x08);
    wr8(DP(0x01), (u8)A);
    int special = 0;
    cmp8((u8)X, 0x00);
    if (fz) {
        lda8(rd8(DP(GAME_MODE)));
        cmp8((u8)A, 0x03);
        if (!fz) {
            cmp8((u8)A, 0x0C);
            if (!fz) special = 1;
        }
    }
    if (special) {
        lda8(rd8(ABS(0x0904)));
        int low = 1;
        if (fz) {
            lda8(rd8(ABS(RIDER_Y)));
            cmp8((u8)A, 0x30);
            low = !fc;
        }
        if (low) { lda8(0x28); wr8(DP(0x01), 0x28); }
    } else {
        lda8(rd8(ABSX(0x0904)));
        if (!fz) {
            lda8(rd8(DP(0x01)));
            cmp8((u8)A, 0xEC);
            if (!fc) { ret(); return; }
        }
    }
    push16(0xCB28); RiderPose_m1x1();
    wr8(DP(0xA8), (u8)A);
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    rep(0x20);
    lda16(rd16(ABSY(0xBAA2)));
    wr16(DP(0x02), A);
    sep(0x20);
    Y = rd8(DP(0xA8)); nz8((u8)Y);
    push8((u8)X);
    X = rd8(DP(0x0D)); nz8((u8)X);
    lda8(rd8(ABSX(RANK_ORDER)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSY(0xCB61)));
    wr8(ABSX(SPRITE_FRAME), (u8)A);
    cmp8((u8)A, 0x05);
    if (fz) {
        lda8(rd8(ABS(RIDER_BUMPED)));
        ora8(rd8(ABS(RIDER_AIR)));
        if (fz) {
            lda8(rd8(DP(RACE_END)));
            cmp8((u8)A, 0x01);
            if (fz) { lda8(0x14); wr8(ABSX(SPRITE_FRAME), 0x14); }
        }
    }
    X = pull8(); nz8((u8)X);
    push16(0xCB5F); DrawBigSprite_m1x1();
    ret();
}
void CrashLand_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    crash_from();
    ret();
}
/* $80:C0BD RiderLandCheck (RTS): rider X in a crash state ($01EE): state 2
 * (tumbling) continues the crash steps unless the player presses right
 * ($017E bit 0 with $7F clear); any other state stops it at speed 0. */
void RiderLandCheck_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_AIR)));
    if (fz) { ret(); return; }
    cmp8((u8)A, 0x02);
    if (!fz) { crash_stop(); ret(); return; }
    lda8(rd8(ABSX(RIDER_INPUT)));
    lda8(lsr8((u8)A));
    if (fc) {
        lda8(rd8(DP(RIDER_TURN)));
        if (fz) { ret(); return; }
    }
    crash_tail();
    ret();
}
/* $80:FB2D BoostOnTile (RTS): unless crashed, the player's speed grows by
 * $0180 ($01FF in the jump contest) up to $0730; then the crash-stop steps
 * for rider X. */
void BoostOnTile_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_AIR)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(GAME_MODE)));
    cmp8((u8)A, 0x14);
    lda8(fz ? 0xFF : 0x80);
    fc = 0; adc8(rd8(ABS(RIDER_SPEED_LO)));
    wr8(ABS(RIDER_SPEED_LO), (u8)A);
    lda8(rd8(ABS(RIDER_SPEED_HI)));
    adc8(0x01);
    wr8(ABS(RIDER_SPEED_HI), (u8)A);
    cmp8((u8)A, 0x07);
    int cap = 0;
    if (fc) {
        if (!fz) cap = 1;
        else {
            lda8(rd8(ABS(RIDER_SPEED_LO)));
            cmp8((u8)A, 0x30);
            cap = fc;
        }
    }
    if (cap) {
        lda8(0x07); wr8(ABS(RIDER_SPEED_HI), 0x07);
        lda8(0x30); wr8(ABS(RIDER_SPEED_LO), 0x30);
    }
    crash_stop();
    ret();
}
/* $80:FBD0 SlowTileCheck (RTS): unless crashed or stopped, mark rider X
 * slow ($08F4 = 1) below speed $0200 and its threshold $0246; then by the
 * tile: $0206 = 2 lands flat (LandFlat), the player on boost ground ($0176
 * = $18) BoostOnTile, otherwise the crash-stop steps. */
void SlowTileCheck_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_AIR)));
    if (!fz) { ret(); return; }
    ldy8(0x00);
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    if (fz) { ret(); return; }
    cmp8((u8)A, 0x02);
    if (!fc) {
        lda8(rd8(ABSX(RIDER_SPEED_LO)));
        cmp8((u8)A, rd8(ABSX(RIDER_SLOW_AT)));
        if (!fc) Y = inc8((u8)Y);
    }
    lda8((u8)Y);
    wr8(ABSX(RIDER_SLOW), (u8)A);
    lda8(rd8(ABSX(RIDER_LANDING)));
    cmp8((u8)A, 0x02);
    if (fz) { LandFlat_m1x1(); return; }
    lda8(rd8(ABS(RIDER_SCRIPT)));
    cmp8((u8)A, 0x18);
    if (fz) { BoostOnTile_m1x1(); return; }
    crash_stop();
    ret();
}
/* $80:FC0E LandFlat (RTS): clear the landing flags $09C4/$09C7, then
 * CrashLand. */
void LandFlat_m1x1(void)
{
    lda8(0x00);
    wr8(ABS(PLAYER_GROUND), 0x00);
    wr8(ABS(0x09C7), 0x00);
    CrashLand_m1x1();
}
/* $80:B0AD CpuReleaseButtons (RTS): a moving CPU rider lets go of the
 * buttons (input $017E & $0F). */
void CpuReleaseButtons_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    if (fz) { ret(); return; }
    lda8(0x0F);
    input_and_a();
    ret();
}
/* $80:B099 CpuThrottle (RTS): a CPU rider below its target speed
 * $0236/$022E holds the throttle (input | $80), otherwise lets go of the
 * buttons (CpuReleaseButtons). */
void CpuThrottle_m1x1(void)
{
    ldy8(0x80);
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    cmp8((u8)A, rd8(ABSX(RIDER_TARGET_HI)));
    if (!fc) { input_or_y(); ret(); return; }
    if (fz) {
        lda8(rd8(ABSX(RIDER_SPEED_LO)));
        cmp8((u8)A, rd8(ABSX(RIDER_TARGET_LO)));
        if (!fc) { input_or_y(); ret(); return; }
    }
    CpuReleaseButtons_m1x1();
}
/* $80:B0B6 CpuReleaseDpad (RTS): input $017E & $F0. */
void CpuReleaseDpad_m1x1(void)
{
    lda8(0xF0);
    input_and_a();
    ret();
}
/* $80:B0BF CpuSteer (RTS): a CPU rider steers toward its wanted lean
 * $0971 (when set): d-pad bit 0 (down) when the lean $01E6 is above it,
 * bit 1 (up) when below. */
void CpuSteer_m1x1(void)
{
    push16(0xB0C1); CpuReleaseDpad_m1x1();
    ldy8(0x01);
    lda8(rd8(ABSX(RIDER_WANT_LEAN)));
    if (fz) { ret(); return; }
    cmp8((u8)A, rd8(ABSX(RIDER_LEAN)));
    if (fz) { ret(); return; }
    if (fc) Y = inc8((u8)Y);
    input_or_y();
    ret();
}
/* $80:CF94 QueuePlaceBadgeHud (RTS): with a place $086E, the place badge
 * at VRAM $185A, attribute $20 (QueuePlaceBadge). */
void QueuePlaceBadgeHud_m1x1(void)
{
    lda8(rd8(ABS(PLAYER_PLACE)));
    if (fz) { ret(); return; }
    lda8(0x5A); wr8(DP(0x00), 0x5A);
    lda8(0x18); wr8(DP(0x01), 0x18);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    QueuePlaceBadge_m1x1();
}
/* $80:CAA1 DrawAllRiders (RTS, X = the last slot): DrawRider for every
 * slot X..0 whose rider ($0950) is active ($01DE), OAM offset $80:BAC7,
 * palette slot $80:BAD7 (the player blinking palette 5 during the coin
 * turbo in races); the player's jump pose $16 adds the mirror image. */
void DrawAllRiders_m1x1(void)
{
    for (;;) {
        wr8(DP(0x0D), (u8)X);
        lda8(rd8(ABSX(0xBAD0)));
        wr8(DP(0x04), (u8)A);
        lda8(rd8(ABS(TURBO_ON)));
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        ora8(rd8(ABSX(RANK_ORDER)));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSX(RANK_ORDER)));
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(RIDER_ACTIVE)));
        if (!fz) {
            lda8(rd8(ABSX(0xBAE0)));
            wr8(DP(0x0B), (u8)A);
            lda8(rd8(ABS(TURBO_ON)));
            if (!fz) {
                lda8(rd8(DP(GAME_MODE)));
                cmp8((u8)A, 0x03);
                if (fz) {
                    cmp8((u8)X, 0x00);
                    if (fz) {
                        lda8(rd8(DP(FRAME_COUNT)));
                        and8(0x02);
                        if (fz) { lda8(0x05); wr8(DP(0x0B), 0x05); }
                    }
                }
            }
            push16(0xCADA); DrawRider_m1x1();
        }
        cmp8((u8)X, 0x00);
        if (fz) {
            lda8(rd8(DP(0xA8)));
            cmp8((u8)A, 0x16);
            if (fz) { push16(0xCAE7); MirrorJumpRider_m1x1(); }
        }
        X = rd8(DP(0x0D)); nz8((u8)X);
        X = dec8((u8)X);
        if (fn) break;
    }
    ret();
}
/* $80:A460 ShowCourseRecord (RTS): with a record course $084C, show its
 * time ($083A-$083C into the race clock) at the VRAM address from
 * $80:97A5 (TickRaceClock), blinking: on even frames the time's queued
 * tiles are blanked ($24). Then $88 = 1. */
void ShowCourseRecord_m1x1(void)
{
    lda8(rd8(ABS(RECORD_COURSE)));
    if (!fz) {
        lda8(rd8(ABS(RECORD_TIME))); wr8(ABS(RACE_MIN), (u8)A);
        lda8(rd8(ABS(RECORD_TIME + 0x01))); wr8(ABS(RACE_SEC), (u8)A);
        lda8(rd8(ABS(RECORD_TIME + 0x02))); wr8(ABS(RACE_FRAMES), (u8)A);
        lda8(rd8(ABS(RECORD_COURSE)));
        lda8(dec8((u8)A));
        lda8(asl8((u8)A));
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(0x97A3))); wr8(DP(0x00), (u8)A);
        lda8(rd8(ABSX(0x97A4))); wr8(DP(0x01), (u8)A);
        push16(0xA489); TickRaceClock_m1x1();
        lda8(rd8(DP(FRAME_COUNT)));
        and8(0x01);
        if (fz) {
            lda8(rd8(LONG(VQ_END)));
            fc = 1; sbc8(0x27);
            X = (u8)A; nz8((u8)X);
            lda8(0x24);
            static const u8 offs[] = { 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C,
                                       0x1A, 0x1C, 0x1E, 0x20, 0x22, 0x24, 0x26 };
            for (unsigned i = 0; i < sizeof offs; i++)
                wr8(LONG(VQ_VMADD + offs[i] + (u8)X), 0x24);
        }
    }
    lda8(0x01);
    wr8(ABS(MODE_TIMER), 0x01);
    ret();
}
/* $80:C2C4 DrawStartFlags (RTS, Y = the flag tile). */
void DrawStartFlags_m1x1(void)
{
    start_flags();
    ret();
}
/* $80:C299 RaceStartLights (RTS): the start of a race. When the lights
 * begin ($3C = 1): unless already running ($92) count ($C3), set the
 * countdown $8A = $40 and play sound $27 (with the coin turbo also $2D and
 * 1); then $92 = 1, $3C = 2. Before the run ($92 clear) ping the APU every
 * 16 frames of $3C. Then the flags with tile $E1/$E2/$E3 by $3C
 * (DrawStartFlags). */
void RaceStartLights_m1x1(void)
{
    lda8(rd8(DP(0x3C)));
    cmp8((u8)A, 0x01);
    if (fz) {
        lda8(rd8(DP(RACE_STARTED)));
        if (fz) {
            wr8(DP(COUNTDOWN), inc8(rd8(DP(COUNTDOWN))));
            lda8(0x40); wr8(DP(TIMER_A), 0x40);
            lda8(0x27);
            push8(0x80); push16(0xC350); QueueSoundB_m1x1();
            lda8(rd8(ABS(TURBO_ON)));
            if (!fz) {
                lda8(0x2D);
                push8(0x80); push16(0xC35B); QueueSoundB_m1x1();
                lda8(0x01);
                push8(0x80); push16(0xC361); QueueSoundB_m1x1();
            }
        }
        lda8(0x01); wr8(DP(RACE_STARTED), 0x01);
        lda8(0x02); wr8(DP(0x3C), 0x02);
    } else {
        Y = rd8(DP(RACE_STARTED)); nz8((u8)Y);
        if (!fz) { ret(); return; }
        Y = rd8(DP(FRAME_COUNT)); nz8((u8)Y);
        and8(0x0F);
        if (fz) { ldy8(0x01); wr8(ABS(APUIO2), 0x01); }
    }
    ldy8(0xE1);
    lda8(rd8(DP(0x3C)));
    cmp8((u8)A, 0x10);
    if (!fc) {
        ldy8(0xE2);
        cmp8((u8)A, 0x08);
        if (!fc) ldy8(0xE3);
    }
    start_flags();
    ret();
}
/* $80:C1FC CrashRecovery (RTS): every 4 frames (phase $0998, counting
 * down) each crashed rider ($01CE) picks a lane: the player in a race lane
 * 4 (sound $25 when not the CPU's turn $7F, and clears $092D), others a
 * random one ($2F & 3) into $0948. A stopped rider (speed high 0) that is
 * not tumbling ($01EE, $0176; the player not on tile $14) gets back up:
 * speed 0, upright lean 6, run-out $090C = column + 8, its shadow, wave
 * pose 2, $01D6 = 1 (the player with sound $3E); a tumbling one slides at
 * $88. Otherwise the lean keeps turning ($01E6 + $01CE) unless held ($3D). */
void CrashRecovery_m1x1(void)
{
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x03);
    cmp8((u8)A, rd8(ABS(CRASH_PHASE)));
    if (!fz) { ret(); return; }
    wr8(ABS(CRASH_PHASE), dec8(rd8(ABS(CRASH_PHASE))));
    lda8(rd8(ABS(CRASH_PHASE)));
    and8(0x03);
    wr8(ABS(CRASH_PHASE), (u8)A);
    crash_loop(0);
}
/* $81:A065 AimRatio (RTL, A = steps): for object X, the direction to the
 * target ($039F/$03A1 vs $027F/$028F) as a step pair: the larger axis gets
 * A, the smaller its share (Bresenham over A steps), signed by direction,
 * into $00 (vertical) and $0A (horizontal). A = 0: $00 = 0. */
void AimRatio_m1x1(void)
{
    wr8(DP(0x0A), (u8)A);
    cmp8((u8)A, 0x00);
    if (fz) { wr8(DP(0x00), 0); retl(); return; }
    push8((u8)X);
    push8((u8)Y);
    rep(0x20);
    ldy8(0x00);
    lda16(rd16(ABS(OBJ_TARGET_Y)));
    fc = 1; sbc16(rd16(ABSX(OBJ_Y)));
    wr16(DP(0x12), A);
    if (fn) Y = inc8((u8)Y);
    wr8(DP(0x02), (u8)Y);
    lda16(rd16(DP(0x12)));
    if (fn) { eor16(0xFFFF); lda16(inc16(A)); }
    wr16(DP(0x10), A);
    ldy8(0x00);
    lda16(rd16(ABS(OBJ_TARGET_X)));
    fc = 1; sbc16(rd16(ABSX(OBJ_X)));
    wr16(DP(0x16), A);
    if (fn) Y = inc8((u8)Y);
    rep(0x20);
    wr8(DP(0x03), (u8)Y);
    lda16(rd16(DP(0x16)));
    if (fn) { eor16(0xFFFF); lda16(inc16(A)); }
    wr16(DP(0x14), A);
    ldy8(0x00);
    cmp16(A, rd16(DP(0x10)));
    if (!fc) {
        Y = inc8((u8)Y);
        push16(A);
        lda16(rd16(DP(0x10)));
        wr16(DP(0x14), A);
        lda16(pull16());
        wr16(DP(0x10), A);
    }
    wr16(DP(0x1E), 0);
    wr16(DP(0x00), 0);
    X = rd8(DP(0x0A)); nz8((u8)X);
    do {
        lda16(rd16(DP(0x1E)));
        fc = 0; adc16(rd16(DP(0x10)));
        cmp16(A, rd16(DP(0x14)));
        if (fc) {
            sbc16(rd16(DP(0x14)));
            wr16(DP(0x00), inc16(rd16(DP(0x00))));
        }
        wr16(DP(0x1E), A);
        X = dec8((u8)X);
        WAIT_TICK();
    } while (!fz);
    sep(0x20);
    lda8((u8)Y);
    if (!fz) {
        lda8(rd8(DP(0x00)));
        push8((u8)A);
        lda8(rd8(DP(0x0A)));
        wr8(DP(0x00), (u8)A);
        lda8(pull8());
        wr8(DP(0x0A), (u8)A);
    }
    lda8(rd8(DP(0x00)));
    Y = rd8(DP(0x02)); nz8((u8)Y);
    if (!fz) { eor8(0xFF); lda8(inc8((u8)A)); wr8(DP(0x00), (u8)A); }
    lda8(rd8(DP(0x0A)));
    Y = rd8(DP(0x03)); nz8((u8)Y);
    if (!fz) { eor8(0xFF); lda8(inc8((u8)A)); wr8(DP(0x0A), (u8)A); }
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    retl();
}
/* $80:C39A SpeedGauge (RTS): returns X = the speedometer needle: 0 unless
 * the gauge is on ($092D) and it is not the CPU's frame ($7F bit 0); else
 * the player's speed $01C6:$01BE / 8, minus 2, divided by 2 (4 when
 * crashed), at least 1, capped at $2F (wobbling to $2E every 8 frames);
 * bit 7 set when the turbo button is up. */
void SpeedGauge_m1x1(void)
{
    ldx8(0x00);
    lda8(rd8(ABS(GAUGE_ON)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(RIDER_TURN)));
    lda8(lsr8((u8)A));
    if (fc) { ret(); return; }
    ldx8(0x02);
    lda8(rd8(ABS(RIDER_SPEED_HI))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABS(RIDER_SPEED_LO))); wr8(DP(0x01), (u8)A);
    do {
        wr8(DP(0x00), lsr8(rd8(DP(0x00))));
        wr8(DP(0x01), ror8(rd8(DP(0x01))));
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x02);
    Y = rd8(ABS(RIDER_AIR)); nz8((u8)Y);
    if (!fz) lda8(0x04);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(DP(0x01)));
    fc = 1; sbc8(0x02);
    int one = fn;
    if (!one) {
        push16(0xC3CC); DivideAX_m1x1();
        lda8((u8)X);
        one = fz;
    }
    if (one) ldx8(0x01);
    cmp8((u8)X, 0x2F);
    if (fc) {
        ldx8(0x2F);
        lda8(rd8(DP(FRAME_COUNT)));
        and8(0x08);
        if (fz) X = dec8((u8)X);
    }
    lda8(rd8(ABS(RIDER_INPUT)));
    lda8(asl8((u8)A));
    if (!fc) {
        lda8((u8)X);
        ora8(0x80);
        X = (u8)A; nz8((u8)X);
    }
    ret();
}
/* $81:B8F4 FallingObjectStep (RTS): object $03A3 flaps every 4 frames
 * ($034F toggles 0/1) and moves (ApplyVelocity), falling ($02AF + 1); when
 * it reaches the ground ($02AF wraps to 0) it lands: next state, pose 2,
 * timer cleared, $036F = 4, sound $49. */
void FallingObjectStep_m1x1(void)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x03);
    if (fz) {
        lda8(rd8(ABSX(OBJ_FRAME)));
        lda8(inc8((u8)A));
        and8(0x01);
        wr8(ABSX(OBJ_FRAME), (u8)A);
    }
    push8(0x81); push16(0xB909); ApplyVelocity_m1x1();
    wr8(ABSX(OBJ_VY), inc8(rd8(ABSX(OBJ_VY))));
    lda8(rd8(ABSX(OBJ_VY)));
    if (!fz) { ret(); return; }
    wr8(ABSX(OBJ_STATE), inc8(rd8(ABSX(OBJ_STATE))));
    lda8(0x02); wr8(ABSX(OBJ_FRAME), 0x02);
    wr8(ABSX(OBJ_STEP), 0);
    lda8(0x04); wr8(ABSX(OBJ_DELAY), 0x04);
    lda8(0x49);
    push8(0x81); push16(0xB927); QueueSoundB_m1x1();
    ret();
}
/* $80:BB5F RiderRunOut (RTS): rider X past the finish ($08C4) and not
 * stopped ($01FE) waves ($099E = 1) and on even frames rolls on ($090C +
 * 1) until it is $80:BB30 (by lane $0948) past its column; then, or when
 * not running out, its finish state advances ($01D6 + 1) and the wave
 * timer $45 = 8. */
void RiderRunOut_m1x1(void)
{
    lda8(rd8(ABSX(0x08C4)));
    int done = fz;
    if (!done) {
        lda8(rd8(ABSX(RIDER_HEIGHT)));
        done = !fz;
    }
    if (!done) {
        lda8(0x01); wr8(ABSX(0x099E), 0x01);
        lda8(rd8(DP(FRAME_COUNT)));
        lda8(lsr8((u8)A));
        if (fc) { ret(); return; }
        wr8(ABSX(RIDER_RUNOUT), inc8(rd8(ABSX(RIDER_RUNOUT))));
        Y = rd8(ABSX(0x0948)); nz8((u8)Y);
        lda8(rd8(ABSX(RIDER_X)));
        fc = 0; adc8(rd8(ABSY(0xBB39)));
        cmp8((u8)A, rd8(ABSX(RIDER_RUNOUT)));
        if (fc) { ret(); return; }
    }
    u8 v = inc8(rd8(ABSX(RIDER_TIMER)));
    wr8(ABSX(RIDER_TIMER), v);
    if (!fz) { lda8(0x08); wr8(DPX(RIDER_LEAN_TIMER), 0x08); }
    ret();
}
/* $80:B997 Mul16 (RTL): $00:$01 * $02:$03 -> $04-$07 with the hardware
 * multiplier. (The high bytes' carry into $07 is the product's own.) */
void Mul16_m1x1(void)
{
    static const u16 back[4] = { 0xB9A3, 0xB9BA, 0xB9D6, 0xB9F2 };
    mul16(0x00, 0x02, 0x04, back);
    retl();
}
void Mul16b_m1x1(void) { mul16b(); }
void Mul16b_m1x0(void) { mul16b(); }
/* $80:86F6 StartEndScreen (RTS): the end of the broadcast: a $0384-frame
 * timer, BG3 at $1900, $1905 = $17, scrolling on ($90, $9A = $84 = 2),
 * sounds $2D and $45, and the end picture's panels queued (the "please
 * wait" panel too before minute 4; the Stadium 2/4 badge with $81:800C). */
void StartEndScreen_m1x1(void)
{
    rep(0x20);
    lda16(0x0384); wr16(DP(MODE_TIMER), 0x0384);
    sep(0x20);
    lda8(0x19); wr8(ABS(BG3SC), 0x19);
    lda8(0x01); wr8(ABS(BG34NBA), 0x01);
    lda8(0x17); wr8(ABS(SH_TM), 0x17);
    lda8(0x01); wr8(DP(SCROLL_HALF), 0x01);
    ldx8(0x02);
    wr8(DP(BG_SCROLL), 0x02);
    wr8(DP(RACE_END), 0x02);
    lda8(0x2D);
    push8(0x80); push16(0x871D); QueueSoundB_m1x1();
    lda8(0x45);
    push8(0x80); push16(0x8723); QueueSoundB_m1x1();
    lda8(rd8(LONG(BCAST_MINUTE)));
    cmp8((u8)A, 0x04);
    if (!fc) queue_rows80(0x06C5, 0x87EB, 0x0218, 0x8745);
    queue_rows80(0x0726, 0x879B, 0x0214, 0x875F);
    queue_rows80(0x0573, 0x884B, 0x030B, 0x8779);
    lda8(rd8(LONG(STADIUM_SELECT)));
    if (!fz) queue_rows80(0x057B, 0x888D, 0x0302, 0x8799);
    ret();
}
/* $81:A747 LoadHdmaScene (RTL, A = scene or $FF): stop HDMA and clear its
 * state ($190x, colour math $190D = $20); for a scene, copy its three
 * line tables ($81:A812/$A905/$A9F8 from $81:A727, lengths $81:A737) to
 * $0692/$0712/$0792 and its colour tables ($81:AAEB/$AB4B/$ABAB from
 * $81:A72F, lengths $81:A73F) to $0452/$0512/$05D2, point HDMA channels
 * 2-4 at the line tables (mode $40 -> $2132), enable them ($190F = $1C,
 * $0812 = 1), clear colour 0 and flag the palette upload. */
void LoadHdmaScene_m1x1(void)
{
    wr8(ABS(HDMAEN), 0);
    wr8(ABS(SH_HDMAEN), 0);
    wr8(ABS(SH_TMW), 0);
    wr8(ABS(SH_TSW), 0);
    wr8(ABS(0x1907), 0);
    wr8(ABS(0x1908), 0);
    wr8(ABS(0x1909), 0);
    wr8(ABS(SH_CGWSEL), 0);
    ldx8(0x20);
    wr8(ABS(SH_CGADSUB), 0x20);
    cmp8((u8)A, 0xFF);
    if (fz) { retl(); return; }
    push8(DB);
    push8(0x81);
    DB = pull8(); nz8(DB);
    rep(0x30);
    wr16(DP(0x00), A);
    and16(0x00FF);
    lda16(asl16(A));
    Y = A; nz16(Y);
    push16(Y);
    lda16(rd16(ABSY(0xA737)));
    wr16(DP(0x02), A);
    X = rd16(ABSY(0xA727)); nz16(X);
    Y = 0; nz16(Y);
    sep(0x20);
    do {
        lda8(rd8(ABSX(0xA812))); wr8(ABSY(0x0692), (u8)A);
        lda8(rd8(ABSX(0xA905))); wr8(ABSY(0x0712), (u8)A);
        lda8(rd8(ABSX(0xA9F8))); wr8(ABSY(0x0792), (u8)A);
        X = inc16(X);
        Y = inc16(Y);
        cmp16(Y, rd16(DP(0x02)));
        WAIT_TICK();
    } while (!fz);
    rep(0x20);
    Y = pull16(); nz16(Y);
    lda16(rd16(ABSY(0xA73F)));
    wr16(DP(0x02), A);
    X = rd16(ABSY(0xA72F)); nz16(X);
    Y = 0; nz16(Y);
    sep(0x20);
    do {
        lda8(rd8(ABSX(0xAAEB))); wr8(ABSY(0x0452), (u8)A);
        lda8(rd8(ABSX(0xAB4B))); wr8(ABSY(0x0512), (u8)A);
        lda8(rd8(ABSX(0xABAB))); wr8(ABSY(0x05D2), (u8)A);
        X = inc16(X);
        Y = inc16(Y);
        cmp16(Y, rd16(DP(0x02)));
        WAIT_TICK();
    } while (!fz);
    X = 0x3240; nz16(X);
    wr16(ABS(DMAP2), 0x3240); wr16(ABS(DMAP3), 0x3240); wr16(ABS(DMAP4), 0x3240);
    X = 0x0692; wr16(ABS(A1T2L), 0x0692);
    X = 0x0712; wr16(ABS(A1T3L), 0x0712);
    X = 0x0792; wr16(ABS(A1T4L), 0x0792); nz16(X);
    lda8(0x00);
    wr8(ABS(A1B2), 0); wr8(ABS(A1B3), 0); wr8(ABS(A1B4), 0);
    wr8(ABS(DASB2), 0); wr8(ABS(DASB3), 0); wr8(ABS(DASB4), 0);
    lda8(0x01); wr8(ABS(0x0812), 0x01);
    lda8(0x1C); wr8(ABS(SH_HDMAEN), 0x1C);
    sep(0x10);
    wr8(ABS(PALETTE_BUF), 0);
    wr8(ABS(PALETTE_BUF + 0x01), 0);
    wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:9002 ShowCoinRank (RTS): in races, on even frames, the coin count
 * box at VRAM $1848: below 6 coins ($1542) one of the six small boxes
 * ($80:90FE + count * $29, attribute $30); from 6 the big box ($80:90AC, or
 * $80:90D5 with the bonus count $199A) with the count ($1542 or $199D) as
 * three digits. */
void ShowCoinRank_m1x1(void)
{
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x01);
    if (!fz) { ret(); return; }
    lda8(rd8(DP(GAME_MODE)));
    cmp8((u8)A, 0x03);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(COIN_TOTAL)));
    cmp8((u8)A, 0x06);
    if (!fc) {
        wr8(ABS(WRMPYA), (u8)A);
        lda8(0x29);
        wr8(ABS(WRMPYB), 0x29);
        rep(0x20);
        lda16(rd16(ABS(RDMPYL)));
        fc = 0; adc16(0x90FC);
        wr16(DP(0x02), A);
        lda16(0x1848); wr16(DP(0x00), 0x1848);
        sep(0x20);
        lda8(0x30); wr8(DP(0x0F), 0x30);
        push16(0x90A8); QueueTileBlock_m1x1();
        ret();
        return;
    }
    ldy8(0x28);
    lda8(rd8(ABS(0x199A)));
    rep(0x20);
    lda16(fz ? 0x90AA : 0x90D3);
    wr16(DP(0x00), A);
    sep(0x20);
    do {
        lda8(rd8((((u32)DB << 16) + rd16(DP(0x00)) + Y) & 0xFFFFFF));
        wr8(ABSY(DIGIT_BUF), (u8)A);
        Y = dec8((u8)Y);
    } while (!fn);
    ldy8(0x00);
    lda8(rd8(ABS(0x199A)));
    if (!fz) lda8(rd8(ABS(0x199D)));
    else lda8(rd8(ABS(COIN_TOTAL)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
        WAIT_TICK();
    }
    wr8(ABS(DIGIT_ROW + 0x0C), (u8)A);
    ora8(0x10); wr8(ABS(0x03F0), (u8)A);
    lda8((u8)Y);
    ldy8(0x00);
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
        WAIT_TICK();
    }
    wr8(ABS(DIGIT_ROW + 0x0A), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x1E), (u8)A);
    lda8((u8)Y);
    wr8(ABS(DIGIT_ROW + 0x08), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x1C), (u8)A);
    rep(0x20);
    lda16(0x1848); wr16(DP(0x00), 0x1848);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    QueueTileBlock_m1x1();
}
/* $80:D824 BonusCourseFrame (RTS): one frame of the bonus course: the
 * time check, scroll, zoom ($0D14/$0D17 from $A2, $0D1A = $10), the end
 * minute, engine sound, countdown sign and BG tile base; the parallax
 * scrolls ($1B00/$1B02/$1B04 from $9C, $B6 once past $0500). During the
 * start countdown ($88) the beeps at $B4/$78/$3C (clearing the sign
 * sprites, sound $26) and at 0 the start ($8A = $40, sound $27). After it
 * the player's input, the 19 course objects (DispatchBy85) and the hidden
 * sprites; when the run has ended ($84 even, not 0) the next mode. */
void BonusCourseFrame_m1x1(void)
{
    push16(0xD826); CheckBonusTime_m1x1();
    push8(0x80); push16(0xD82A); SetJumpScroll_m1x1();
    lda8(rd8(DP(ZOOM)));
    eor8(0xFF);
    lda8(inc8((u8)A));
    fc = 0; adc8(0x01);
    wr8(ABS(0x0D14), (u8)A);
    lda8(rd8(DP(ZOOM)));
    fc = 0; adc8(0x80);
    wr8(ABS(0x0D17), (u8)A);
    lda8(0x10); wr8(ABS(0x0D1A), 0x10);
    push16(0xD845); EndAtBonusMinute_m1x1();
    push16(0xD848); SendSpeedToApu_m1x1();
    push16(0xD84B); DrawCountdownSignB_m1x1();
    push8(0x80); push16(0xD84F); SetBgTileBase_m1x1();
    rep(0x20);
    lda16(rd16(DP(SCROLL_X)));
    lda16(lsr16(A));
    wr16(ABS(0x1B02), A);
    wr16(ABS(0x1B04), A);
    lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A));
    wr16(ABS(0x1B00), A);
    lda16(rd16(ABS(0x00B6)));
    if (fz) {
        lda16(rd16(ABS(0x1B02)));
        cmp16(A, 0x0500);
        if (fc) wr16(ABS(0x00B6), inc16(rd16(ABS(0x00B6))));
    }
    lda16(rd16(DP(MODE_TIMER)));
    if (!fz) {
        wr16(DP(MODE_TIMER), dec16(rd16(DP(MODE_TIMER))));
        lda16(rd16(DP(MODE_TIMER)));
        cmp16(A, 0x00B4);
        int beep = fz;
        if (!beep) { cmp16(A, 0x0078); beep = fz; }
        if (!beep) { cmp16(A, 0x003C); beep = fz; }
        if (beep) {
            sep(0x20);
            ldx8(0x3C);
            do {
                lda8(0xF0);
                wr8(ABSX(OAM_BUF + 0x101), 0xF0);
                X = dec8((u8)X); X = dec8((u8)X); X = dec8((u8)X); X = dec8((u8)X);
            } while (!fn);
            lda8(0x26);
            push8(0x80); push16(0xD8B6); QueueSoundB_m1x1();
            wr8(DP(COUNTDOWN), inc8(rd8(DP(COUNTDOWN))));
            ret();
            return;
        }
        cmp16(A, 0x0000);
        if (fz) {
            lda16(0x0040); wr16(DP(TIMER_A), 0x0040);
            sep(0x20);
            lda8(0x27);
            push8(0x80); push16(0xD89C); QueueSoundB_m1x1();
            wr8(DP(COUNTDOWN), inc8(rd8(DP(COUNTDOWN))));
        }
        sep(0x20);
        ret();
        return;
    }
    sep(0x20);
    wr8(ABS(SPRITES_FROZEN), 0);
    lda8(rd8(DP(0x54)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(RACE_END)));
    if (!fz) {
        lda8(lsr8((u8)A));
        if (!fc) {
            lda8(0x02); wr8(DP(MODE_HOLD), 0x02);
            lda8(0x00); wr8(DP(SUB_STEP), 0x00);
            NextMode_m1x1();
            return;
        }
    }
    lda8(rd8(DP(P1_HELD_HI)));
    wr8(ABS(RIDER_INPUT), (u8)A);
    lda8(0x00); wr8(DP(SUB_STEP), 0x00);
    do {
        push16(0xD8D5); DispatchBy85_m1x1();
        wr8(DP(SUB_STEP), inc8(rd8(DP(SUB_STEP))));
        lda8(rd8(DP(SUB_STEP)));
        cmp8((u8)A, 0x13);
    } while (!fc);
    ldx8(0x09);
    do {
        lda8((u8)X);
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(0xF0); wr8(ABSY(OAM_BUF + 0xC1), 0xF0);
        X = dec8((u8)X);
    } while (!fn);
    ret();
}
/* $81:CA20 ScaleJumpCount (RTL, either A width): Hundreds, then the jump
 * count $1569 times the result less one into $0C. */
void ScaleJumpCount_m1x1(void)
{
    push8(getP());
    push16(0xCA23); Hundreds_m1x1();
    scale_jump_tail();
}
void ScaleJumpCount_m0x1(void)
{
    push8(getP());
    push16(0xCA23); Hundreds_m0x1();
    scale_jump_tail();
}
/* $81:F9C8 BsxFindFile (M=0 X=0, RTL): look up the broadcast directory
 * ($7E:9BEC, $7E:9BF2 entries) for the channel $13D3 and file $13D5: on a
 * match copy its 13-byte record to $13F3 and return C clear; otherwise C
 * set and $7E:13C9 = 3. Entries that are not the channel are skipped by
 * their size (count * 13 + 3 bytes). */
void BsxFindFile_m0x0(void)
{
    push8(getP());
    sep(0x20);
    rep(0x10);
    push8(DB);
    push16(0x7E7E);
    DB = pull8(); nz8(DB);
    DB = pull8(); nz8(DB);
    lda8(rd8(LONG(0x7E9BF2)));
    wr8(LONG(0x7E004B), (u8)A);
    X = 0x0008; nz16(X);
    for (;;) {
        rep(0x20);
        lda16(rd16(LONGX(0x7E9BEC)));
        cmp16(A, rd16(LONG(0x0013D3)));
        sep(0x20);
        if (fz) {
            X = inc16(X); X = inc16(X);
            lda8(rd8(LONGX(0x7E9BEC)));
            wr8(LONG(0x7E0047), (u8)A);
            X = inc16(X);
            for (;;) {
                rep(0x20);
                lda16(rd16(LONGX(0x7E9BED)));
                cmp16(A, rd16(LONG(0x0013D5)));
                sep(0x20);
                if (fz) {
                    Y = 0; nz16(Y);
                    do {
                        lda8(rd8(LONGX(0x7E9BEC)));
                        push16(X);                       /* PHX; TYX */
                        wr8(LONG(0x0013F3 + Y), (u8)A);
                        X = pull16();                    /* PLX */
                        X = inc16(X);
                        Y = inc16(Y);
                        cmp16(Y, 0x000D);
                    } while (!fz);
                    DB = pull8(); nz8(DB);
                    setP(pull8());
                    fc = 0;
                    retl();
                    return;
                }
                rep(0x20);
                lda16(X);
                fc = 0; adc16(0x000D);
                X = A; nz16(X);
                sep(0x20);
                u8 n = dec8(rd8(ABS(RIDER_LEAN_TIMER + 0x02)));
                wr8(ABS(RIDER_LEAN_TIMER + 0x02), n);
                WAIT_TICK();
                if (fz) break;
            }
        } else {
            rep(0x20);
            lda16(rd16(LONGX(0x7E9BEE)));
            and16(0x00FF);
            wr16(LONG(0x7E0047), A);
            lda16(asl16(A));
            adc16(rd16(LONG(0x7E0047)));
            lda16(asl16(A));
            lda16(asl16(A));
            adc16(rd16(LONG(0x7E0047)));
            wr16(ABS(RIDER_LEAN_TIMER + 0x02), X);
            adc16(rd16(LONG(0x7E0047)));
            adc16(0x0003);
            X = A; nz16(X);
            sep(0x20);
        }
        u8 c = dec8(rd8(ABS(RIDER_LEAN_TIMER + 0x06)));
        wr8(ABS(RIDER_LEAN_TIMER + 0x06), c);
        WAIT_TICK();
        if (fz) break;
    }
    lda8(0x03);
    wr8(LONG(0x7E13C9), 0x03);
    DB = pull8(); nz8(DB);
    setP(pull8());
    fc = 1;
    retl();
}
/* $80:946F ShowItemPoints (RTS): the items' points, $1548 * 10 + $1547 * 8
 * + $1546 * 6 + $1545 * 4 + $1544 * 2, as four digits plus a zero
 * ($03D0-$03D4, lower row +$10), the bonus digits added (AddBcdDigits),
 * leading zeros blank, size $25, queued at VRAM $1977. */
void ShowItemPoints_m1x1(void)
{
    rep(0x20);
    lda16(rd16(ABS(0x1548)));
    and16(0x00FF);
    lda16(asl16(A));
    wr16(DP(0x00), A);
    lda16(asl16(A)); lda16(asl16(A));
    fc = 0; adc16(rd16(DP(0x00)));
    wr16(DP(0x02), A);
    lda16(rd16(ABS(0x1547)));
    and16(0x00FF);
    lda16(asl16(A)); lda16(asl16(A)); lda16(asl16(A));
    fc = 0; adc16(rd16(DP(0x02)));
    wr16(DP(0x02), A);
    lda16(rd16(ABS(0x1546)));
    and16(0x00FF);
    lda16(asl16(A));
    wr16(DP(0x00), A);
    lda16(asl16(A));
    fc = 0; adc16(rd16(DP(0x00)));
    fc = 0; adc16(rd16(DP(0x02)));
    wr16(DP(0x02), A);
    lda16(rd16(ABS(0x1545)));
    and16(0x00FF);
    lda16(asl16(A)); lda16(asl16(A));
    fc = 0; adc16(rd16(DP(0x02)));
    wr16(DP(0x02), A);
    lda16(rd16(ABS(0x1544)));
    and16(0x00FF);
    lda16(asl16(A));
    fc = 0; adc16(rd16(DP(0x02)));
    wr16(DP(0x02), A);
    static const u16 pow10[3] = { 1000, 100, 10 };
    for (int i = 0; i < 3; i++) {
        ldy8(0x00);
        for (;;) {
            cmp16(A, pow10[i]);
            if (!fc) break;
            sbc16(pow10[i]);
            Y = inc8((u8)Y);
            WAIT_TICK();
        }
        wr16(DP(i < 2 ? 0x02 : 0x04), A);
        lda16((u16)(Y & 0xFF));
        wr16(ABS(DIGIT_ROW + i), A);
        ora16(0x0010);
        wr16(ABS(DIGIT_ROW + 0x05 + i), A);
        if (i < 2) lda16(rd16(DP(0x02)));
    }
    lda16(0x1977); wr16(DP(0x00), 0x1977);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(rd8(DP(0x04)));
    wr8(ABS(DIGIT_ROW + 0x03), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x08), (u8)A);
    lda8(0x00); wr8(ABS(DIGIT_ROW + 0x04), 0x00);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x09), (u8)A);
    push16(0x952B); AddBcdDigits_m1x1();
    lda8(0x25); wr8(ABS(DIGIT_BUF), 0x25);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    ldx8(0x01);
    for (;;) {
        lda8(rd8(ABSX(DIGIT_BUF)));
        if (!fz) break;
        lda8(0xFF);
        wr8(ABSX(DIGIT_BUF), 0xFF);
        wr8(ABSX(DIGIT_ROW + 0x04), 0xFF);
        X = inc8((u8)X);
        cmp8((u8)X, 0x05);
        if (fc) break;
    }
    VramQueueTiles_m1x1();
}
/* $80:FC2C UpdateLanes (RTS): every rider's lane position $01F6 moves by
 * its steering $01AE (not while landing from a jump, $0176 = $14/$15),
 * stops at a target lane $19A6 or one of the lane lines $80:F6AB, bounces
 * between rows 8 and $39 (the edge flips the steering), and the jump
 * states $020E settle (landing with CrashLand). Then the player's d-pad
 * (up $08 / down $04) steers, with sound $22 on a press or every 4 frames. */
void UpdateLanes_m1x1(void)
{
    ldx8(0x07);
    for (;;) {
        lda8(rd8(ABSX(RIDER_SCRIPT)));
        cmp8((u8)A, 0x15);
        int move;
        if (fz) {
            lda8(rd8(ABSX(RIDER_TIMER)));
            move = !fz;
        } else {
            cmp8((u8)A, 0x14);
            move = !fz;
        }
        if (move) {
            lda8(rd8(ABSX(RIDER_LANE)));
            fc = 0; adc8(rd8(ABSX(RIDER_STEER)));
            wr8(ABSX(RIDER_LANE), (u8)A);
        }
        lda8(rd8(ABSX(0x199E)));
        if (!fz) {
            lda8(rd8(ABSX(RIDER_LANE)));
            cmp8((u8)A, rd8(ABSX(0x19A6)));
            if (!fz) goto next;
            wr8(ABSX(0x199E), 0);
            goto stop;
        }
        ldy8(0x07);
        do {
            lda8(rd8(ABSX(RIDER_LANE)));
            cmp8((u8)A, rd8(ABSY(0xF678)));
            if (fz) goto stop;
            Y = dec8((u8)Y);
        } while (!fn);
        lda8(rd8(ABSX(RIDER_JUMP)));
        if (!fz) {
            cmp8((u8)A, 0x01);
            if (fz) {
                lda8(rd8(ABSX(RIDER_LANE)));
                cmp8((u8)A, 0x20);
                if (!fc) {
                    lda8(0x04); wr8(ABSX(RIDER_JUMP), 0x04);
                    lda8(rd8(ABSX(RIDER_SCRIPT)));
                    int land = fz;
                    if (!land) { cmp8((u8)A, 0x12); land = fz; }
                    if (!land) { cmp8((u8)A, 0x10); land = fz; }
                    if (!land) { cmp8((u8)A, 0x0D); land = fz; }
                    if (!land) goto next;
                    lda8(0x00); wr8(ABSX(RIDER_HEIGHT), 0x00);
                    push16(0xFC9A); CrashLand_m1x1();
                    {
                        u8 v = dec8(rd8(ABSX(RIDER_JUMP)));
                        wr8(ABSX(RIDER_JUMP), v);
                    }
                    if (!fz) goto next;
                    /* falls on with A as CrashLand left it */
                    cmp8((u8)A, 0x03);
                    if (fz) goto rows;
                    goto drift;
                }
                goto rows;
            }
            cmp8((u8)A, 0x03);
            if (fz) goto rows;
        drift:
            lda8(rd8(ABSX(RIDER_LANE)));
            cmp8((u8)A, 0x20);
            if (fc) wr8(ABSX(RIDER_LANE), dec8(rd8(ABSX(RIDER_LANE))));
        }
    rows:
        lda8(rd8(ABSX(RIDER_LANE)));
        cmp8((u8)A, 0x08);
        if (!fc) {
            lda8(rd8(ABSX(RIDER_TIMER)));
            if (fz) lda8(0x07);
            else {
                lda8(rd8(ABSX(RIDER_LANE)));
                cmp8((u8)A, 0x02);
                if (!fc) lda8(0x01);
            }
            wr8(ABSX(RIDER_LANE), (u8)A);
            cmp8((u8)X, 0x00);
            if (fz) goto flip;
            goto edge;
        }
        cmp8((u8)A, 0x3A);
        if (!fc) goto next;
        lda8(0x39); wr8(ABSX(RIDER_LANE), 0x39);
    edge:
        lda8(rd8(ABSX(RIDER_TIMER)));
        ora8(rd8(ABSX(OVERHEAT_LOCK)));
        if (!fz) goto next;
    flip:
        lda8(rd8(ABSX(RIDER_STEER)));
        if (fz) lda8(0xFF);
        eor8(0xFE);
        wr8(ABSX(RIDER_STEER), (u8)A);
        goto next;
    stop:
        lda8(0x00);
        wr8(ABSX(RIDER_STEER), 0x00);
    next:
        X = dec8((u8)X);
        if (fn) break;
    }
    lda8(rd8(DP(RACE_STARTED)));
    if (fz) { ret(); return; }
    lda8(rd8(ABS(RIDER_BUMPED)));
    ora8(rd8(ABS(OVERHEAT_LOCK)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(RIDER_AIR)));
    if (!fz) {
        lda8(rd8(ABS(RIDER_SLOW)));
        cmp8((u8)A, 0x02);
        if (!fz) { ret(); return; }
        wr8(ABS(RIDER_SLOW), inc8(rd8(ABS(RIDER_SLOW))));
    }
    lda8(rd8(ABS(RIDER_TIMER)));
    if (!fz) {
        cmp8((u8)A, 0x05);
        if (!fz) { ret(); return; }
    }
    Y = rd8(ABS(RIDER_STEER)); nz8((u8)Y);
    lda8(rd8(ABS(RIDER_INPUT)));
    and8(0x04);
    if (!fz) lda8(0xFF);
    else {
        lda8(rd8(ABS(RIDER_INPUT)));
        and8(0x08);
        if (fz) { ret(); return; }
        lda8(0x01);
    }
    wr8(ABS(RIDER_STEER), (u8)A);
    lda8(rd8(DP(P1_PRESS_HI)));
    and8(0x0C);
    if (fz) {
        lda8(rd8(DP(FRAME_COUNT)));
        and8(0x03);
        if (!fz) { ret(); return; }
    }
    lda8(0x22);
    push8(0x80); push16(0xFD49); QueueSoundB_m1x1();
    ret();
}
/* $82:8BFE UnpackToWram (RTS, A = entry): unpack stream A of the set at
 * $00 (pointer table $82:96A9, three bytes each) to $7E:4000. */
void UnpackToWram_m1x1(void)
{
    rep(0x30);
    lda16(asl16(A));
    fc = 0; adc16(rd16(DP(0x00)));
    Y = A; nz16(Y);
    sep(0x20);
    lda8(rd8(ABSY(0x96A9))); wr8(DP(0x08), (u8)A);
    lda8(rd8(ABSY(0x96AA))); wr8(DP(0x09), (u8)A);
    lda8(rd8(ABSY(0x96AB))); wr8(DP(0x0A), (u8)A);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x40); wr8(DP(0x01), 0x40);
    lda8(0x7E); wr8(DP(0x02), 0x7E);
    unpack_stream();
}
/* $82:8BD8 UnpackToLowRam (RTS, A = entry): as UnpackToWram, to $00:1C00
 * (the palette buffer). */
void UnpackToLowRam_m1x1(void)
{
    lda8(asl8((u8)A));
    fc = 0; adc8(rd8(DP(0x00)));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0x96A9))); wr8(DP(0x08), (u8)A);
    lda8(rd8(ABSY(0x96AA))); wr8(DP(0x09), (u8)A);
    lda8(rd8(ABSY(0x96AB))); wr8(DP(0x0A), (u8)A);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x1C); wr8(DP(0x01), 0x1C);
    lda8(0x00); wr8(DP(0x02), 0x00);
    unpack_stream();
}
/* $81:A053 AimVelocity (RTL, A = steps): object X's velocity toward its
 * target from AimRatio ($02AF vertical, $029F horizontal). */
void AimVelocity_m1x1(void)
{
    push8(0x81); push16(0xA056); AimRatio_m1x1();
    lda8(rd8(DP(0x00))); wr8(ABSX(OBJ_VY), (u8)A);
    lda8(rd8(DP(0x0A))); wr8(ABSX(OBJ_VX), (u8)A);
    retl();
}
/* $80:DD34 StartBonusTry (RTS): count the try ($1E95), load the bonus HDMA
 * scene (3 before minute $28, else 2) and set $3A = $20. */
void StartBonusTry_m1x1(void)
{
    wr8(ABS(0x1E95), inc8(rd8(ABS(0x1E95))));
    lda8(rd8(LONG(BCAST_MINUTE)));
    cmp8((u8)A, 0x28);
    lda8(fc ? 0x02 : 0x03);
    push8(0x80); push16(0xDD48); LoadHdmaScene_m1x1();
    lda8(0x20); wr8(DP(0x3A), 0x20);
    ret();
}
/* $80:E945 JumpResultCount (RTS): the wave, then the jump count
 * (ShowJumpCount, into which it falls). */
void JumpResultCount_m1x1(void)
{
    if (!jump_result_wave(0xE949)) { ret(); return; }
    ShowJumpCount_m1x1();
}
/* $80:E9A1 JumpResultBonus (RTS): the wave, then the bonus points
 * (ShowBonusPoints, into which it falls). */
void JumpResultBonus_m1x1(void)
{
    if (!jump_result_wave(0xE9A5)) { ret(); return; }
    ShowBonusPoints_m1x1();
}
/* $80:EA47 JumpResultWait (RTS): the waving player; after $B4 frames
 * ($88) or a button on either pad, back to the jump contest (mode $14,
 * sprites frozen). */
void JumpResultWait_m1x1(void)
{
    ldx8(0x00);
    push16(0xEA4B); DrawAllRiders_m1x1();
    lda8(0x19); wr8(ABS(SPRITE_FRAME), 0x19);
    rep(0x20);
    wr16(DP(MODE_TIMER), inc16(rd16(DP(MODE_TIMER))));
    lda16(rd16(DP(MODE_TIMER)));
    cmp16(A, 0x00B4);
    if (!fz) {
        sep(0x20);
        lda8(rd8(DP(P1_PRESS_HI)));
        ora8(rd8(DP(P1_PRESS_LO)));
        if (fz) { ret(); return; }
    }
    sep(0x20);
    lda8(0x14); wr8(DP(GAME_MODE), 0x14);
    wr8(ABS(SPRITES_FROZEN), inc8(rd8(ABS(SPRITES_FROZEN))));
    ChangeMode_m1x1();
}
/* $80:8F24 StartPlayerSelect (RTS): reset the select state ($1E8B, the
 * five name letters from $80:8F64), step on, no fade ($1923 = 1), BG1 at
 * $0100, clear the held buttons, $198E = $50, the select screen itself,
 * a $0708-frame timer, an empty track and no HDMA scene. */
void StartPlayerSelect_m1x1(void)
{
    wr8(ABS(0x1E8B), 0);
    ldx8(0x04);
    do {
        lda8(rd8(ABSX(0x8F62)));
        wr8(ABSX(0x1554), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    wr8(DP(MODE_ACTIVE), inc8(rd8(DP(MODE_ACTIVE))));
    lda8(0x01); wr8(ABS(FADE_STATE), 0x01);
    lda8(0x01); wr8(ABS(BG1SC), 0x01);
    wr8(DP(P1_HELD_HI), 0);
    wr8(DP(P1_HELD_LO), 0);
    lda8(0x50); wr8(ABS(SELECT_ROW), 0x50);
    push16(0x8F49); SelectMain_m1x1();
    rep(0x20);
    lda16(0x0708); wr16(ABS(MODE_TIMER), 0x0708);
    lda16(0x0000); wr16(LONG(TRACK_BUF), 0x0000);
    sep(0x20);
    lda8(0x00);
    push8(0x80); push16(0x8F60); LoadHdmaScene_m1x1();
    ret();
}
/* $80:E8EE JumpResultTally (RTS): the waving player; every 8 frames ($CF)
 * the pose frame $CE steps (frames $80:E92F); after the fourth, next
 * step ($CD), timers $88 = $80, $8A = $8C = $20, and the total
 * $1995 = ScaleJumpCount's $0C + the rank points $80:F155. */
void JumpResultTally_m1x1(void)
{
    ldx8(0x00);
    push16(0xE8F2); DrawAllRiders_m1x1();
    u8 t = dec8(rd8(DP(0xCF)));
    wr8(DP(0xCF), t);
    lda8(t);
    if (fz) {
        wr8(DP(WAVE_FRAME), inc8(rd8(DP(WAVE_FRAME))));
        lda8(rd8(DP(WAVE_FRAME)));
        cmp8((u8)A, 0x04);
        if (fc) {
            wr8(DP(WAVE_FRAME), dec8(rd8(DP(WAVE_FRAME))));
            wr8(DP(JUMP_BOARD_STEP), inc8(rd8(DP(JUMP_BOARD_STEP))));
            rep(0x20);
            lda16(0x0080); wr16(DP(MODE_TIMER), 0x0080);
            lda16(0x0020); wr16(DP(TIMER_A), 0x0020);
            lda16(0x0020); wr16(DP(TIMER_B), 0x0020);
            push8(0x80); push16(0xE919); ScaleJumpCount_m0x1();
            Y = rd8(ABS(JUMP_RANK)); nz8((u8)Y);
            lda16(rd16(ABSY(0xF122)));
            and16(0x00FF);
            fc = 0; adc16(rd16(DP(0x0C)));
            wr16(ABS(0x1995), A);
            sep(0x20);
        }
        lda8(0x08); wr8(DP(0xCF), 0x08);
    }
    X = rd8(DP(WAVE_FRAME)); nz8((u8)X);
    lda8(rd8(ABSX(0xE938)));
    wr8(ABS(SPRITE_FRAME), (u8)A);
    ret();
}
/* $80:91FA ResetRaceBoard (RTS): every rider's place $086E = $7D * 8
 * (high byte 0), the place badge, the six track rows $09D3-$0CD2 cleared
 * to $3B, sprite 0 parked, and the streamer's start ($0276 = $34, column
 * $14 on page 5, $0279 = $0A). */
void ResetRaceBoard_m1x1(void)
{
    lda8(rd8(DP(FINAL_RACE)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    ldx8(0x07);
    do {
        wr8(ABSX(PLAYER_PLACE), (u8)A);
        wr8(ABSX(0x086F), 0);
        X = dec8((u8)X);
    } while (!fn);
    push16(0x920C); QueuePlaceBadgeHud_m1x1();
    ldx8(0x7F);
    lda8(0x3B);
    do {
        wr8(ABSX(TRACK_ROWS), 0x3B); wr8(ABSX(TRACK_ROWS + 0x80), 0x3B); wr8(ABSX(TRACK_ROWS + 0x100), 0x3B);
        wr8(ABSX(TRACK_ROWS + 0x180), 0x3B); wr8(ABSX(TRACK_ROWS + 0x200), 0x3B); wr8(ABSX(TRACK_ROWS + 0x280), 0x3B);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0xBF); wr8(ABS(OAM_BUF + 0x01), 0xBF);
    lda8(0xF7); wr8(ABS(OAM_BUF), 0xF7);
    lda8(0xFF); wr8(ABS(OAM_BUF + 0x02), 0xFF);
    lda8(0x34); wr8(ABS(TRACK_ROW_PTR), 0x34);
    lda8(0x14); wr8(ABS(TRACK_VRAM_COL), 0x14);
    lda8(0x0A); wr8(ABS(0x0279), 0x0A);
    lda8(0x05); wr8(ABS(TRACK_VRAM_HALF), 0x05);
    ret();
}
/* $80:A187 StoreEntryNumber (RTL): the three entered digits ($1570-$1572)
 * as a number (100s, 10s, 1s), whose bits are spread over the code bytes
 * $0850-$0854 (two bits each into bits 4-5). */
void StoreEntryNumber_m1x1(void)
{
    rep(0x20);
    wr16(DP(0x10), 0);
    wr16(DP(0x12), 0);
    sep(0x20);
    acc_mul(rd8(ABS(ENTRY_DIGITS)), 0x0064, 0xA19F);
    acc_mul(rd8(ABS(ENTRY_DIGITS + 0x01)), 0x000A, 0xA1B7);
    lda8(rd8(ABS(ENTRY_DIGITS + 0x02)));
    wr8(DP(0x04), (u8)A);
    wr8(DP(0x05), 0); wr8(DP(0x06), 0); wr8(DP(0x07), 0);
    push16(0xA1C8); Add32_m1x1();
    lda8(rd8(DP(0x10)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    and8(0x30); ora8(rd8(ABS(ENTRY_CODE + 0x04))); wr8(ABS(ENTRY_CODE + 0x04), (u8)A);
    lda8(rd8(DP(0x10)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    and8(0x30); ora8(rd8(ABS(ENTRY_CODE + 0x03))); wr8(ABS(ENTRY_CODE + 0x03), (u8)A);
    lda8(rd8(DP(0x10)));
    and8(0x30); ora8(rd8(ABS(ENTRY_CODE + 0x02))); wr8(ABS(ENTRY_CODE + 0x02), (u8)A);
    lda8(rd8(DP(0x10)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    and8(0x30); ora8(rd8(ABS(ENTRY_CODE + 0x01))); wr8(ABS(ENTRY_CODE + 0x01), (u8)A);
    lda8(rd8(DP(0x11)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    and8(0x30); ora8(rd8(ABS(ENTRY_CODE))); wr8(ABS(ENTRY_CODE), (u8)A);
    retl();
}
/* $80:A079 LoadPackNumber (RTL): the five decimal digits at $11:5730
 * (the memory pack) as a number, stored as nibbles in $0850-$0853 and the
 * top bits (<< 4, & $1E) in $0855. */
void LoadPackNumber_m1x1(void)
{
    rep(0x20);
    wr16(DP(0x10), 0);
    wr16(DP(0x12), 0);
    sep(0x20);
    acc_mul(rd8(LONG(SAVE_BEST_SCORE)), 0x2710, 0xA092);
    acc_mul(rd8(LONG(SAVE_BEST_SCORE + 0x01)), 0x03E8, 0xA0AB);
    acc_mul(rd8(LONG(SAVE_BEST_SCORE + 0x02)), 0x0064, 0xA0C4);
    acc_mul(rd8(LONG(SAVE_BEST_SCORE + 0x03)), 0x000A, 0xA0DD);
    lda8(rd8(LONG(SAVE_BEST_SCORE + 0x04)));
    wr8(DP(0x04), (u8)A);
    wr8(DP(0x05), 0); wr8(DP(0x06), 0); wr8(DP(0x07), 0);
    push16(0xA0EF); Add32_m1x1();
    lda8(rd8(DP(0x12)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    and8(0x1E); wr8(ABS(ENTRY_CODE + 0x05), (u8)A);
    lda8(rd8(DP(0x11)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(ABS(ENTRY_CODE), (u8)A);
    lda8(rd8(DP(0x11)));
    and8(0x0F); wr8(ABS(ENTRY_CODE + 0x01), (u8)A);
    lda8(rd8(DP(0x10)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(ABS(ENTRY_CODE + 0x02), (u8)A);
    lda8(rd8(DP(0x10)));
    and8(0x0F); wr8(ABS(ENTRY_CODE + 0x03), (u8)A);
    retl();
}
/* $80:D65F StartBonusCourse (RTS): the bonus course set-up: rider
 * palettes; 64 columns of track rows cleared (tile $FC, heights and flags
 * 0, the six rows $3B); the streamer's start; the player alone on screen
 * (column $30, wheel column 9, lean 6, page 1, active, $94 = 2); its HDMA
 * scene (3 before minute $28, else 1); the player drawn at height $B0,
 * then set to $B8; the parallax tables $0D11-$0D25 from $80:D782 on HDMA
 * channels 5-7; zoom $9E = $A2 = $0100, a $012C-frame countdown, the
 * parallax scrolls cleared and HDMA on ($190F = $9C). */
void StartBonusCourse_m1x1(void)
{
    push8(0x80); push16(0xD662); LoadRiderPalettes_m1x1();
    ldx8(0x3F);
    do {
        lda8(0xFC); wr8(ABSX(TRACK_ROWS), 0xFC);
        lda8(0x00);
        wr8(ABSX(TRACK_ROWS + 0x40), 0); wr8(ABSX(TRACK_ROWS + 0x80), 0); wr8(ABSX(TRACK_ROWS + 0xC0), 0);
        lda8(0x3B);
        wr8(ABSX(TRACK_ROWS + 0x180), 0x3B); wr8(ABSX(TRACK_ROWS + 0x1C0), 0x3B); wr8(ABSX(TRACK_ROWS + 0x200), 0x3B);
        wr8(ABSX(TRACK_ROWS + 0x240), 0x3B); wr8(ABSX(TRACK_ROWS + 0x280), 0x3B); wr8(ABSX(TRACK_ROWS + 0x2C0), 0x3B);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x34); wr8(ABS(TRACK_ROW_PTR), 0x34);
    lda8(0x0A); wr8(ABS(0x0279), 0x0A);
    lda8(0x05); wr8(ABS(TRACK_VRAM_HALF), 0x05);
    lda8(0x74); wr8(ABS(TRACK_VRAM_COL), 0x74);
    lda8(0x08); wr8(ABS(RIDER_WHEEL), 0x08);
    lda8(0x09); wr8(ABS(RIDER_COLUMN), 0x09);
    lda8(0x30); wr8(ABS(RIDER_X), 0x30);
    lda8(0x06); wr8(ABS(RIDER_LEAN), 0x06);
    lda8(0x01);
    wr8(ABS(RIDER_PAGE), 0x01); wr8(ABS(RIDER_ACTIVE), 0x01); wr8(ABS(RIDER_BAND), 0x01);
    lda8(asl8((u8)A));
    wr8(DP(0x94), (u8)A);
    lda8(0x17); wr8(ABS(SH_TM), 0x17);
    lda8(rd8(LONG(BCAST_MINUTE)));
    cmp8((u8)A, 0x28);
    lda8(fc ? 0x01 : 0x03);
    push8(0x80); push16(0xD6D8); LoadHdmaScene_m1x1();
    ldx8(0x00);
    lda8(0xB0); wr8(ABS(GROUND_LINE), 0xB0); wr8(ABS(RIDER_Y), 0xB0);
    push16(0xD6E5); DrawAllRiders_m1x1();
    lda8(0xB8); wr8(ABS(GROUND_LINE), 0xB8); wr8(ABS(RIDER_Y), 0xB8);
    ldx8(0x14);
    do {
        lda8(rd8(ABSX(0xD78B)));
        wr8(ABSX(0x0D11), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    static const u8 dma[3][4] = { { 0x42, 0x0F, 0x11, 0x0D }, { 0x42, 0x10, 0x18, 0x0D }, { 0x40, 0x08, 0x1F, 0x0D } };
    for (int c = 0; c < 3; c++) {
        u16 b = (u16)(0x4370 - 0x10 * c);
        for (int k = 0; k < 4; k++) { lda8(dma[c][k]); wr8(ABS(b + k), dma[c][k]); }
        lda8(0x00); wr8(ABS(b + 4), 0x00);
        lda8(0x00); wr8(ABS(b + 7), 0x00);
    }
    rep(0x20);
    lda16(0x0100); wr16(DP(BG1_VSCROLL), 0x0100); wr16(DP(ZOOM), 0x0100);
    lda16(0x012C); wr16(DP(MODE_TIMER), 0x012C);
    wr16(ABS(0x1B00), 0); wr16(ABS(0x1B02), 0); wr16(ABS(0x1B04), 0); wr16(ABS(0x1B06), 0);
    lda16(0x0000); wr16(ABS(0x1B20), 0x0000);
    lda16(0x006E); wr16(ABS(0x1B22), 0x006E);
    sep(0x20);
    lda8(0x43); wr8(ABS(0x1B40), 0x43);
    lda8(0x43); wr8(ABS(0x1B41), 0x43);
    lda8(0x9C); wr8(ABS(SH_HDMAEN), 0x9C);
    ret();
}
/* $80:9FFC ShowEntryCode (RTL): build the entry code (memory pack number,
 * broadcast date, entered number, the BS-X flags, the course bits) and
 * show its eleven bytes $0850-$085A as tiles ((b & $38) << 2 + (b & 7) *
 * 2) in three groups: 4 at VRAM $0871, 4 at $0875, 4 at $0879 (sizes $14,
 * $14, $13; attributes 7, 3, $0B). */
void ShowEntryCode_m1x1(void)
{
    push8(0x80); push16(0x9FFF); LoadPackNumber_m1x1();
    push8(0x80); push16(0xA003); ReadBroadcastDate_m1x1();
    push8(0x80); push16(0xA007); StoreEntryNumber_m1x1();
    push8(0x80); push16(0xA00B); MergeBsxFlags_m1x1();
    push8(0x80); push16(0xA00F); CountCourseBits_m1x1();
    ldx8(0x0A);
    do {
        lda8(rd8(ABSX(ENTRY_CODE)));
        and8(0x38);
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        wr8(DP(0x00), (u8)A);
        lda8(rd8(ABSX(ENTRY_CODE)));
        and8(0x07);
        lda8(asl8((u8)A));
        fc = 0; adc8(rd8(DP(0x00)));
        wr8(ABSX(DIGIT_ROW), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    static const struct { u16 size_at, vram, src; u8 size, attr; u16 back; } g[3] = {
        { 0x03CF, 0x0871, 0x03CF, 0x14, 0x07, 0xA043 },
        { 0x03D3, 0x0875, 0x03D3, 0x14, 0x03, 0xA05D },
        { 0x03D7, 0x0879, 0x03D7, 0x13, 0x0B, 0xA077 },
    };
    for (int i = 0; i < 3; i++) {
        lda8(g[i].size); wr8(ABS(g[i].size_at), g[i].size);
        rep(0x20);
        lda16(g[i].vram); wr16(DP(0x00), g[i].vram);
        lda16(g[i].src); wr16(DP(0x02), g[i].src);
        sep(0x20);
        lda8(g[i].attr); wr8(DP(0x0F), g[i].attr);
        push16(g[i].back); VramQueueTiles_m1x1();
    }
    retl();
}
/* $81:9F3A SteerObjectStep (RTL, Y = flags): object X's velocity toward
 * its target (AimVelocity) with sign-extended high bytes; flag bit 0
 * reverses it (flee), bit 1 updates its facing, bit 2 moves it in X only,
 * bit 3 in both. An axis that passes the target ($039F/$03A1) snaps onto
 * it; C set when both did. */
void SteerObjectStep_m1x1(void)
{
    wr8(DP(0x04), (u8)Y);
    push8(0x81); push16(0x9F3F); AimVelocity_m1x1();
    ldy8(0x00);
    lda8(rd8(ABSX(OBJ_VX)));
    if (fn) Y = dec8((u8)Y);
    lda8((u8)Y); wr8(ABSX(OBJ_VX + 0x01), (u8)A);
    ldy8(0x00);
    lda8(rd8(ABSX(OBJ_VY)));
    if (fn) Y = dec8((u8)Y);
    lda8((u8)Y); wr8(ABSX(OBJ_VY + 0x01), (u8)A);
    rep(0x20);
    wr16(DP(0x04), lsr16(rd16(DP(0x04))));      /* LSR $04 with A 16-bit */
    if (fc) {
        lda16(rd16(ABSX(OBJ_VX))); eor16(0xFFFF); lda16(inc16(A)); wr16(ABSX(OBJ_VX), A);
        lda16(rd16(ABSX(OBJ_VY))); eor16(0xFFFF); lda16(inc16(A)); wr16(ABSX(OBJ_VY), A);
    }
    sep(0x20);
    wr8(DP(0x04), lsr8(rd8(DP(0x04))));
    if (fc) { push8(0x81); push16(0x9F7B); ObjectFacing_m1x1(); }
    wr8(DP(0x04), lsr8(rd8(DP(0x04))));
    if (fc) { push8(0x81); push16(0x9F83); ApplyVelocityX_m1x1(); }
    wr8(DP(0x04), lsr8(rd8(DP(0x04))));
    if (fc) { push8(0x81); push16(0x9F8B); ApplyVelocity_m1x1(); }
    rep(0x20);
    wr16(DP(0x00), 0);
    lda16(rd16(ABSX(OBJ_VX)));
    if (!fz) {
        lda16(rd16(ABSX(OBJ_X)));
        fc = 1; sbc16(rd16(ABS(OBJ_TARGET_X)));
        eor16(rd16(ABSX(OBJ_VX)));
        if (!fn) {
            lda16(rd16(ABS(OBJ_TARGET_X))); wr16(ABSX(OBJ_X), A);
            wr16(DP(0x00), inc16(rd16(DP(0x00))));
        }
    }
    lda16(rd16(ABSX(OBJ_Y)));
    if (!fz) {
        lda16(rd16(ABSX(OBJ_Y)));
        fc = 1; sbc16(rd16(ABS(OBJ_TARGET_Y)));
        eor16(rd16(ABSX(OBJ_VY)));
        if (!fn) {
            lda16(rd16(ABS(OBJ_TARGET_Y))); wr16(ABSX(OBJ_Y), A);
            wr16(DP(0x00), inc16(rd16(DP(0x00))));
        }
    }
    sep(0x20);
    lda8(rd8(DP(0x00)));
    cmp8((u8)A, 0x02);
    fc = fz ? 1 : 0;
    retl();
}
/* $80:8899 Mode02_SegmentCard (RTS): the course card waits for its minute
 * ($80:88AF by course $7C * 2 + final $7D), then the next mode. */
void Mode02_SegmentCard_m1x1(void)
{
    lda8(rd8(DP(COURSE_SLOT)));
    lda8(asl8((u8)A));
    fc = 0; adc8(rd8(DP(FINAL_RACE)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(LONG(BCAST_MINUTE)));
    cmp8((u8)A, rd8(ABSX(0x88AD)));
    if (!fc) { ret(); return; }
    NextMode_m1x1();
}
/* $80:9DC6 Mode11 (RTS): wait $88 frames, then the end screen (mode $0F,
 * fade, a $20-frame timer). */
void Mode11_m1x1(void)
{
    rep(0x20);
    u16 t = dec16(rd16(DP(MODE_TIMER)));
    wr16(DP(MODE_TIMER), t);
    sep(0x20);
    if (!fz) { ret(); return; }
    lda8(0x0F); wr8(DP(GAME_MODE), 0x0F);
    wr8(DP(MODE_ACTIVE), 0);
    wr8(DP(MODE_STEP), 0);
    wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
    rep(0x20);
    lda16(0x0020); wr16(DP(MODE_TIMER), 0x0020);
    sep(0x20);
    ret();
}
/* $80:98B0 Mode0F_EndScreen (RTS): the broadcast-over check, then the
 * end screen's step $0450 through the table $80:98C8 (JMP ($0000)). */
void Mode0F_EndScreen_m1x1(void)
{
    push16(0x98B2); BsxEndIfFlagged_m1x1();
    lda8(rd8(ABS(END_STEP)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0x98C6))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSX(0x98C7))); wr8(DP(0x01), (u8)A);
    r816_call_pc(0x800000u | rd16(LONG(0x000000)));
}
/* $80:F2FD DemoInputStep (RTS): the attract demo's recorded input: the
 * debug flag lets Start leave; on the first frame load the first entry
 * ($9C:D200 input, $9C:D300 duration, $63 = 0); each frame feed the input
 * to the player ($017E, $098E) for its duration $0CE3, then the next
 * entry; a duration of $F8 or more ends the demo through exit $FF - it.
 * C clear while it plays. */
void DemoInputStep_m1x1(void)
{
    lda8(rd8(LONG(DEBUG_ENABLED)));
    if (!fz) {
        lda8(rd8(DP(P1_PRESS_HI)));
        and8(0x10);
        if (!fz) { ldy8(0x00); demo_exit(); return; }
    }
    lda8(rd8(DP(RACE_STARTED)));
    if (fz) {
        wr8(DP(FRAME_COUNT), 0);
        lda8(rd8(LONG(0x9CD200))); wr8(ABS(0x197C), (u8)A);
        lda8(rd8(LONG(0x9CD300))); wr8(ABS(DEMO_TIME), (u8)A);
        lda8(0x01); wr8(ABS(DEMO_INDEX), 0x01);
    } else {
        lda8(rd8(ABS(DEMO_TIME)));
        if (!fz) {
            wr8(ABS(DEMO_TIME), dec8(rd8(ABS(DEMO_TIME))));
            X = rd8(ABS(DEMO_INDEX)); nz8((u8)X);
            X = dec8((u8)X);
            lda8(rd8(LONGX(0x9CD200)));
            wr8(ABS(RIDER_INPUT), (u8)A);
            wr8(ABS(DEMO_PAD), (u8)A);
        } else {
            X = rd8(ABS(DEMO_INDEX)); nz8((u8)X);
            wr8(ABS(DEMO_INDEX), inc8(rd8(ABS(DEMO_INDEX))));
            lda8(rd8(LONGX(0x9CD300)));
            cmp8((u8)A, 0xF8);
            if (fc) {
                eor8(0xFF);
                Y = (u8)A; nz8((u8)Y);
                demo_exit();
                return;
            }
            wr8(ABS(DEMO_TIME), (u8)A);
            lda8(rd8(LONGX(0x9CD200)));
            wr8(ABS(RIDER_INPUT), (u8)A);
            wr8(ABS(DEMO_PAD), (u8)A);
        }
    }
    fc = 0;
    ret();
}
/* $80:F495 TitlePushStart (RTS): from minute 6 the PUSH START panel
 * blinks (one of three frames by $C5, tiles from $80:F50D at VRAM $18F1);
 * Start (sound $2D) leaves through exit 0. */
void TitlePushStart_m1x1(void)
{
    lda8(rd8(LONG(BCAST_MINUTE)));
    cmp8((u8)A, 0x06);
    if (!fc) { ret(); return; }
    wr8(DP(0xC5), inc8(rd8(DP(0xC5))));
    lda8(rd8(DP(0xC5)));
    and8(0x20);
    lda8(lsr8((u8)A));
    wr8(DP(0x00), (u8)A);
    wr8(DP(0x00), lsr8(rd8(DP(0x00))));
    wr8(DP(0x00), lsr8(rd8(DP(0x00))));
    fc = 0; adc8(rd8(DP(0x00)));
    wr8(DP(0x00), lsr8(rd8(DP(0x00))));
    wr8(DP(0x00), lsr8(rd8(DP(0x00))));
    fc = 0; adc8(rd8(DP(0x00)));
    rep(0x20);
    and16(0x00FF);
    fc = 0; adc16(0xF4DA);
    wr16(DP(0x02), A);
    lda16(0x18F1); wr16(DP(0x00), 0x18F1);
    sep(0x20);
    push16(0xF4C8); VramQueueTiles20_m1x1();
    lda8(rd8(DP(P1_PRESS_HI)));
    and8(0x10);
    if (fz) { ret(); return; }
    lda8(0x2D); wr8(ABS(APUIO0), 0x2D);
    ldy8(0x00);
    demo_exit();
}
/* $80:C1A4 OverheatStep (RTS): while the engine is overheated ($60 or
 * $0940): above step 8 the stopped player cools down standing (speed 0, or
 * $C0 while landing; steering toward lane $80:BACF) with dust puffs; at
 * step 8 the overheat ends (sound $2D, heat $0990 = $08CC = 5, finish
 * state 5 unless set). */
void OverheatStep_m1x1(void)
{
    lda8(rd8(DP(OVERHEATED)));
    ora8(rd8(ABS(OVERHEAT_LOCK)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(OVERHEATED)));
    cmp8((u8)A, 0x08);
    if (fz) {
        lda8(0x00); wr8(ABS(OVERHEAT_LOCK), 0x00);
        lda8(0x2D);
        push8(0x80); push16(0xC1E8); QueueSoundB_m1x1();
        lda8(0x05);
        wr8(ABS(ENGINE_HEAT), 0x05);
        wr8(ABS(RIDER_COUNTDOWN), 0x05);
        Y = rd8(ABS(RIDER_TIMER)); nz8((u8)Y);
        if (fz) wr8(ABS(RIDER_TIMER), (u8)A);
        ret();
        return;
    }
    if (!fc) { ret(); return; }
    lda8(rd8(ABS(RIDER_SPEED_HI)));
    ora8(rd8(ABS(RIDER_BUMPED)));
    ora8(rd8(ABS(RIDER_TIMER)));
    if (fz) {
        lda8(rd8(ABS(RIDER_SCRIPT)));
        if (!fz) {
            lda8(0xC0); wr8(ABS(RIDER_SPEED_LO), 0xC0);
        } else {
            wr8(ABS(RIDER_SPEED_LO), (u8)A);
            ldx8(0x01);
            lda8(rd8(ABS(RIDER_LANE)));
            cmp8((u8)A, rd8(ABS(0xBAD8)));
            if (fz) X = dec8((u8)X);
            wr8(ABS(RIDER_STEER), (u8)X);
        }
    }
    overheat_dust();
    ret();
}
/* $80:FE07 ItemFlashState (RTS): the item-hit flash $5E: cleared when the
 * race is over or overheated; while it runs returns X = 6 (and A = $40 on
 * its odd steps from $0D) or X = 0 on even steps from $0D, Y = 7. */
void ItemFlashState_m1x1(void)
{
    lda8(rd8(DP(RACE_END)));
    ora8(rd8(DP(OVERHEATED)));
    if (!fz) { lda8(0x00); wr8(DP(0x5E), 0x00); ret(); return; }
    lda8(rd8(DP(0x5E)));
    if (fz) { ret(); return; }
    ldx8(0x00);
    ldy8(0x07);
    lda8(rd8(DP(0x5E)));
    cmp8((u8)A, 0x0D);
    if (fc) {
        lda8(lsr8((u8)A));
        if (!fc) { ret(); return; }
        lda8(0x40);
    }
    ldx8(0x06);
    ret();
}
/* $80:B2DF VramQueueTilesLong (RTL): VramQueueTiles for bank $81/$82. */
void VramQueueTilesLong_m1x1(void)
{
    push16(0xB2E1); VramQueueTiles_m1x1();
    retl();
}
/* $80:B43A QueueTileRowsLong (RTL): QueueTileRows for other banks. */
void QueueTileRowsLong_m1x1(void)
{
    push16(0xB43C); QueueTileRows_m1x1();
    retl();
}
/* $80:F8CE LanePieceTables (RTS, Y = lane): the pointer $03:$04 to lane
 * Y's track-piece row ($80:F6C0 / $80:F6C6). */
void LanePieceTables_m1x1(void)
{
    lda8(rd8(ABSY(0xF68D))); wr8(DP(0x03), (u8)A);
    lda8(rd8(ABSY(0xF693))); wr8(DP(0x04), (u8)A);
    ret();
}
/* $80:F8C4 LanePieceTablesLong (RTL): LanePieceTables with DB = $80. */
void LanePieceTablesLong_m1x1(void)
{
    push8(DB);
    lda8(0x80);
    push8(0x80);
    DB = pull8(); nz8(DB);
    push16(0xF8CB); LanePieceTables_m1x1();
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:F8A8 LanePieceAt (RTS, Y = lane, $08 = column). */
void LanePieceAt_m1x1(void) { lane_piece(); }
/* $80:F8A3 RiderPieceAt (RTS, Y = lane): LanePieceAt at rider X's wheel
 * column $0266. */
void RiderPieceAt_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_COLUMN)));
    wr8(DP(0x08), (u8)A);
    lane_piece();
}
/* $80:B05A IsPieceFlat (RTS): C clear when the piece type A is $1F. */
void IsPieceFlat_m1x1(void)
{
    cmp8((u8)A, 0x1F);
    fc = fz ? 0 : 1;
    ret();
}
/* $80:B057 RiderOnFlat (RTS, Y = lane): RiderPieceAt, then IsPieceFlat. */
void RiderOnFlat_m1x1(void)
{
    push16(0xB059); RiderPieceAt_m1x1();
    IsPieceFlat_m1x1();
}
/* $80:C052 HeightAbove (RTS): A - rider X's jump height $01FE - its lane
 * $01F6 - $09C7 - 8. */
void HeightAbove_m1x1(void)
{
    fc = 1;
    sbc8(rd8(ABSX(RIDER_HEIGHT)));
    sbc8(rd8(ABSX(RIDER_LANE)));
    sbc8(rd8(ABS(0x09C7)));
    fc = 1;
    sbc8(0x08);
    ret();
}
/* $80:C04F GroundHeightAbove (RTS): HeightAbove from the ground line
 * $0CD5. */
void GroundHeightAbove_m1x1(void)
{
    lda8(rd8(ABS(GROUND_LINE)));
    HeightAbove_m1x1();
}
void PlayerColumnCheck_m1x1(void) { player_column(0xDC79); }
void PlayerColumnCheckB_m1x1(void) { player_column(0xEF09); }
void CpuSteerUp_m1x1(void) { steer_lane(1, 0); }
void CpuSteerDown_m1x1(void) { steer_lane(0, 0); }
/* $81:C883 (RTL): queue the tiles at $03CF (size $20) to VRAM $19F3 and
 * the two rows at $81:C8B5 ($020B) to $19E7. */
void QueueResultsTiles_m1x1(void)
{
    rep(0x20);
    lda16(0x19F3); wr16(DP(0x00), 0x19F3);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push8(0x81); push16(0xC898); VramQueueTilesLong_m1x1();
    rep(0x20);
    lda16(0x19E7); wr16(DP(0x00), 0x19E7);
    lda16(0xC8B5); wr16(DP(0x02), 0xC8B5);
    ldx8(0x81); wr8(DP(0x04), 0x81);
    lda16(0x020B); wr16(DP(0x10), 0x020B);
    sep(0x20);
    push8(0x81); push16(0xC8B3); QueueTileRowsLong_m1x1();
    retl();
}
/* $82:84EE (RTL): for each of the four players flagged in $1E8D, queue
 * its marker tiles ($82:8518, size $2C) at the VRAM address $82:851D. */
void QueuePlayerMarkers_m1x1(void)
{
    push8(DB);
    lda8(0x82); push8(0x82);
    DB = pull8(); nz8(DB);
    ldy8(0x03);
    do {
        lda8(rd8(ABSY(0x1E8D)));
        if (!fz) {
            lda8((u8)Y);
            lda8(asl8((u8)A));
            X = (u8)A; nz8((u8)X);
            rep(0x20);
            lda16(rd16(ABSX(0x851D)));
            wr16(DP(0x00), A);
            lda16(0x8518); wr16(DP(0x02), 0x8518);
            sep(0x20);
            lda8(0x2C); wr8(DP(0x0F), 0x2C);
            push8(0x82); push16(0x8512); VramQueueTilesLong_m1x1();
        }
        Y = dec8((u8)Y);
    } while (!fn);
    DB = pull8(); nz8(DB);
    retl();
}
/* $81:8BEB (RTL): fill column $0F of the six lane rows (5 down to 0) from
 * the course buffer, two bytes a lane from entry $1A down: the piece
 * $7F:000E,X (skipping $FC ramp tops) and its height $7F:200E,X ($0180
 * further on in the row). */
void FillLaneColumn_m1x1(void)
{
    ldx8(0x05);
    lda8(0x1A); wr8(DP(0x0C), 0x1A);
    do {
        Y = X; nz8((u8)Y);
        push8(0x81); push16(0x8BF5); LanePieceTablesLong_m1x1();
        wr8(DP(0x0A), (u8)X);
        for (;;) {
            X = rd8(DP(0x0C)); nz8((u8)X);
            lda8(rd8(LONGX(VQ0_DATA)));
            cmp8((u8)A, 0xFC);
            if (!fz) break;
            wr8(DP(0x0C), dec8(rd8(DP(0x0C))));
            wr8(DP(0x0C), dec8(rd8(DP(0x0C))));
            lda8(rd8(DP(0x0C)));
            if (fn) break;
        }
        Y = rd8(DP(0x0F)); nz8((u8)Y);
        wr8((((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF, (u8)A);
        rep(0x30);
        lda16(Y);
        fc = 0; adc16(0x0180);
        Y = A; nz16(Y);
        sep(0x20);
        lda8(rd8(LONGX(0x7F200E)));
        wr8((((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF, (u8)A);
        sep(0x10);
        wr8(DP(0x0C), dec8(rd8(DP(0x0C))));
        wr8(DP(0x0C), dec8(rd8(DP(0x0C))));
        X = rd8(DP(0x0A)); nz8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
    retl();
}
void CourseColumn_m1x1(void) { course_column(0xD316, 0xD31F, 0xDB99); }
void CourseColumnB_m1x1(void) { course_column(0xE038, 0xE049, 0xEE27); }
/* $81:9B5D (RTL): object X's map tile (row $028F, column $027F in pixels)
 * from the map $81:9B7F (+$A0 unless $FF), queued as one tile word
 * (palette $11) at VRAM $1840 + offset. */
void QueueMapTile_m1x1(void)
{
    rep(0x20);
    lda16(rd16(ABSX(OBJ_Y)));
    and16(0x00F8);
    lda16(asl16(A)); lda16(asl16(A));
    wr16(DP(0x00), A);
    lda16(rd16(ABSX(OBJ_X)));
    lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A));
    fc = 0; adc16(rd16(DP(0x00)));
    Y = (u8)A; nz8((u8)Y);
    fc = 0; adc16(0x1840);
    wr16(DP(0x00), A);
    lda16(0x03A5); wr16(DP(0x02), 0x03A5);
    lda16(rd16(ABSY(0x9B7F)));
    and16(0x00FF);
    cmp16(A, 0x00FF);
    if (!fz) { fc = 0; adc16(0x00A0); }
    A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
    ora16(0x0011);
    wr16(ABS(0x03A5), A);
    sep(0x20);
    lda8(0x1C); wr8(DP(0x0F), 0x1C);
    push8(0x81); push16(0x9B9D); VramQueueTilesLong_m1x1();
    retl();
}
/* $81:C6B4 (RTS): the banner at VRAM $194B. Once the score $9C (nonzero)
 * reaches $1985 it holds frame 2 (and is left alone from $197F's level 2
 * on); before that it alternates frames 0 / 2 every 32 frames ($63). Two rows ($020A) of
 * the frame at $81:C6B0. */
void QueueBlinkBanner_m1x1(void)
{
    rep(0x20);
    lda16(0x194B); wr16(DP(0x00), 0x194B);
    lda16(rd16(DP(SCROLL_X)));
    int alternate = fz;
    if (!alternate) { cmp16(A, rd16(ABS(0x1985))); alternate = !fc; }
    if (!alternate) {
        lda16(rd16(ABS(JUMP_RUN_STEP)));
        and16(0x000F);
        cmp16(A, 0x0002);
        if (fc) { sep(0x20); ret(); return; }
        ldx8(0x02);
    } else {
        lda16(0x194B); wr16(DP(0x00), 0x194B);
        lda16(rd16(DP(FRAME_COUNT)));
        and16(0x0020);
        lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A));
        X = (u8)A; nz8((u8)X);
    }
    lda16(rd16(ABSX(0xC6B0)));
    wr16(DP(0x02), A);
    lda16(0x020A); wr16(DP(0x10), 0x020A);
    sep(0x20);
    lda8(0x81); wr8(DP(0x04), 0x81);
    push8(0x81); push16(0xC6F5); QueueTileRowsLong_m1x1();
    sep(0x20);
    ret();
}
/* $82:83A1 (RTL): the five digits at [$18] (leading zeros as blank $0C)
 * as a two-row number ($03D0 / $03D6 +$10, a trailing 0 / $10) queued
 * with size $26 at VRAM $00. X and Y are kept. */
void QueueFiveDigits_m1x1(void)
{
    push8((u8)X); push8((u8)Y);
    ldx8(0x00); ldy8(0x00);
    do {
        lda8(rd8((ILDP(0x18) + Y) & 0xFFFFFF));
        int blank = 0;
        if (fz) { cmp8((u8)X, 0x00); blank = fz; }
        if (blank) lda8(0x0C);
        else X = inc8((u8)X);
        wr8(ABSY(DIGIT_ROW), (u8)A);
        ora8(0x10);
        wr8(ABSY(DIGIT_ROW + 0x06), (u8)A);
        Y = inc8((u8)Y);
        cmp8((u8)Y, 0x05);
    } while (!fc);
    wr8(ABS(DIGIT_ROW + 0x05), 0);
    lda8(0x10); wr8(ABS(DIGIT_ROW + 0x0B), 0x10);
    lda8(0x26); wr8(ABS(DIGIT_BUF), 0x26);
    rep(0x20);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x01); wr8(DP(0x0F), 0x01);
    push8(0x82); push16(0x83DE); VramQueueTilesLong_m1x1();
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    retl();
}
/* $82:83E2 (RTL, Y = last index): the Y + 1 digits at ($02) as a two-row
 * number (lower row +$10), leading zeros blanked ($FF), size Y + 1 | $20,
 * queued at VRAM $00. */
void QueueDigitsAt_m1x1(void)
{
    lda8((u8)Y);
    wr8(DP(0x04), (u8)A);
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    do {
        lda8(rd8((((u32)DB << 16) + rd16(DP(0x02)) + Y) & 0xFFFFFF));
        wr8(ABSY(DIGIT_ROW), (u8)A);
        ora8(0x10);
        wr8(ABSX(DIGIT_ROW + 0x01), (u8)A);
        X = dec8((u8)X);
        Y = dec8((u8)Y);
    } while (!fn);
    lda8(rd8(DP(0x04)));
    lda8(inc8((u8)A));
    ora8(0x20);
    wr8(ABS(DIGIT_BUF), (u8)A);
    ldx8(0x00);
    lda8(rd8(DP(0x04)));
    lda8(inc8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    for (;;) {
        lda8(rd8(ABSX(DIGIT_ROW)));
        if (!fz) break;
        lda8(0xFF);
        wr8(ABSX(DIGIT_ROW), 0xFF);
        wr8(ABSY(DIGIT_ROW), 0xFF);
        Y = inc8((u8)Y);
        X = inc8((u8)X);
        cmp8((u8)X, rd8(DP(0x04)));
        if (fc) break;
    }
    rep(0x20);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    push8(0x82); push16(0x8422); VramQueueTilesLong_m1x1();
    retl();
}
/* $80:C9A0 (RTS): the player (not crashed, not off the track) running into
 * the ball: lane $01F6 - 3 .. + 4 around the ball's $097C, X within $12 of
 * $090F, ball ($08C7) out and the player at screen page 1 ($01A1), and the
 * ball low enough (GroundHeightAbove - 8 below speed $01B6). Then the
 * player stops (flags $01CE / $01B1 / $01D9 / $091F) and A = 1. */
void BallHit_m1x1(void)
{
    lda8(rd8(ABS(RIDER_TIMER)));
    ora8(rd8(ABS(OVERHEAT_LOCK)));
    if (!fz) { ret(); return; }
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABS(RIDER_PAGE + 0x03)));
    cmp8((u8)A, 0x01);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(BALL_OUT)));
    if (fz) { ret(); return; }
    lda8(rd8(ABS(RIDER_LANE)));
    fc = 1; sbc8(0x03);
    cmp8((u8)A, rd8(ABS(BALL_LANE)));
    if (fc) { ret(); return; }
    adc8(0x07);
    cmp8((u8)A, rd8(ABS(BALL_LANE)));
    if (!fc) { ret(); return; }
    lda8(rd8(ABS(RIDER_X)));
    cmp8((u8)A, rd8(ABS(RIDER_RUNOUT + 0x03)));
    if (fc) { ret(); return; }
    adc8(0x12);
    cmp8((u8)A, rd8(ABS(RIDER_RUNOUT + 0x03)));
    if (!fc) { ret(); return; }
    push16(0xC9D8); GroundHeightAbove_m1x1();
    fc = 1; sbc8(0x08);
    cmp8((u8)A, rd8(ABS(RIDER_Y)));
    if (fc) { ret(); return; }
    ldy8(0xFF); wr8(ABS(RIDER_BUMPED), 0xFF);
    Y = inc8((u8)Y); wr8(ABS(RIDER_STEER + 0x03), (u8)Y);
    Y = inc8((u8)Y); wr8(ABS(RIDER_TIMER + 0x03), (u8)Y);
    Y = inc8((u8)Y); wr8(ABS(0x091F), (u8)Y);
    lda8(0x01);
    ret();
}
/* $80:9565 (RTS): the six digits $1549 become $0CE5 - $1549 (digit by
 * digit, then borrowing), queued at VRAM $19B6 (QueueDigitsAt, size $20). */
void ShowDigitDifference_m1x1(void)
{
    ldx8(0x05);
    do {
        lda8(rd8(ABSX(SCORE_DIGITS)));
        fc = 1; sbc8(rd8(ABSX(SCORE_COPY)));
        wr8(ABSX(SCORE_COPY), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    ldx8(0x05);
    do {
        lda8(rd8(ABSX(SCORE_COPY)));
        if (fn) {
            fc = 0; adc8(0x0A);
            wr8(ABSX(SCORE_COPY), (u8)A);
            wr8(ABSX(0x1548), dec8(rd8(ABSX(0x1548))));
        }
        X = dec8((u8)X);
    } while (!fn);
    rep(0x20);
    lda16(0x1549); wr16(DP(0x02), 0x1549);
    lda16(0x19B6); wr16(DP(0x00), 0x19B6);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    ldy8(0x05);
    push8(0x80); push16(0x959E); QueueDigitsAt_m1x1();
    ret();
}
/* $80:9947 (RTS): when the timer $88 runs out, the five digits at
 * $00:0CE5 at VRAM $0975, $88 = $20, and the end screen's next steps
 * ($0450 = 4, $0451 = 5; or 3 when $085D is set). */
void EndScreenScore_m1x1(void)
{
    u8 t = dec8(rd8(DP(MODE_TIMER)));
    wr8(DP(MODE_TIMER), t);
    if (!fz) { ret(); return; }
    rep(0x20);
    lda16(0x0CE5); wr16(DP(0x18), 0x0CE5);
    lda16(0x0975); wr16(DP(0x00), 0x0975);
    sep(0x20);
    lda8(0x00); wr8(DP(0x1A), 0x00);
    push8(0x80); push16(0x9960); QueueFiveDigits_m1x1();
    lda8(0x20); wr8(DP(MODE_TIMER), 0x20);
    lda8(rd8(ABS(0x085D)));
    if (fz) {
        lda8(0x04); wr8(ABS(END_STEP), 0x04);
        lda8(0x05); wr8(ABS(0x0451), 0x05);
    } else {
        lda8(0x03); wr8(ABS(END_STEP), 0x03);
    }
    ret();
}
/* $81:984C (RTS): object $03A3 follows its path ($81:9C3F, four bytes a
 * point: target $039F / $03A1) with SteerObjectStep; on arriving it moves
 * to the next point, and at the $8000 end marker resets the point to $60
 * and steps its state $030F. Otherwise its map tile (unless $0280) and
 * its velocity direction. */
void ObjectFollowPath_m1x1(void)
{
    rep(0x30);
    X = rd16(ABS(OBJ_CURRENT)); nz16(X);
    lda16(rd16(ABSX(OBJ_STEP)));
    lda16(asl16(A)); lda16(asl16(A));
    Y = A; nz16(Y);
    lda16(rd16(ABSY(0x9C3F))); wr16(ABS(OBJ_TARGET_X), A);
    lda16(rd16(ABSY(0x9C41))); wr16(ABS(OBJ_TARGET_Y), A);
    sep(0x30);
    ldy8(0x0C);
    lda8(0x20);
    push8(0x81); push16(0x986C); SteerObjectStep_m1x1();
    if (fc) {
        rep(0x30);
        wr16(ABSX(OBJ_STEP), inc16(rd16(ABSX(OBJ_STEP))));
        lda16(rd16(ABSX(OBJ_STEP)));
        lda16(asl16(A)); lda16(asl16(A));
        Y = A; nz16(Y);
        lda16(rd16(ABSY(0x9C3F)));
        cmp16(A, 0x8000);
        sep(0x30);
        if (fz) {
            rep(0x20);
            lda16(0x0060); wr16(ABSX(OBJ_STEP), 0x0060);
            sep(0x20);
            wr8(ABSX(OBJ_STATE), inc8(rd8(ABSX(OBJ_STATE))));
            ret();
            return;
        }
    }
    lda8(rd8(ABSX(OBJ_X + 0x01)));
    if (fz) { push8(0x81); push16(0x989A); QueueMapTile_m1x1(); }
    push8(0x81); push16(0x989E); VelocityDirection_m1x1();
    ret();
}
/* $80:AF90 (RTS, X = rider): CPU riders 0-3 on screen page 1 decide on a
 * lane change. Blocked by a rider ahead in its lane: let go of the
 * buttons and, on a $1F piece in the right half of the screen, steer
 * away (by its pad bit 0 / $0216). Otherwise, not already steering: at
 * a $0C piece move toward the middle lanes; else, unless on a $1E
 * piece, at X $80-$EF try a lane up. */
void CpuLaneDecision_m1x1(void)
{
    cmp8((u8)X, 0x04);
    if (fc) { ret(); return; }
    lda8(rd8(ABSX(RIDER_PAGE)));
    cmp8((u8)A, 0x01);
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_BAND)));
    wr8(DP(0x02), (u8)A);
    ldy8(0x02);
    push16(0xAFA4); RiderAhead_m1x1();
    if (fc) {
        push16(0xAFA9); CpuReleaseButtons_m1x1();
        lda8(rd8(ABSX(RIDER_TILE)));
        cmp8((u8)A, 0x1F);
        if (!fz) { ret(); return; }
        lda8(rd8(ABSX(RIDER_X)));
        lda8(asl8((u8)A));
        if (!fc) { ret(); return; }
        lda8(rd8(DPX(RANDOM_LFSR)));
        lda8(lsr8((u8)A));
        if (fc) steer_by_e6();
        else steer_lane(1, 0);
        return;
    }
    cmp8((u8)X, 0x04);
    if (fc) { ret(); return; }
    lda8(rd8(ABSX(RIDER_STEER)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_TILE)));
    cmp8((u8)A, 0x0C);
    if (fz) {
        lda8(rd8(ABSX(RIDER_LANE_NUM)));
        cmp8((u8)A, 0x03);
        steer_lane(fc, 1);
        return;
    }
    cmp8((u8)A, 0x1E);
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_PAGE)));
    cmp8((u8)A, 0x01);
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_X)));
    cmp8((u8)A, 0xF0);
    if (fc) { ret(); return; }
    cmp8((u8)A, 0x80);
    if (!fc) { ret(); return; }
    steer_lane(1, 0);
}
void PlayerStop_m1x1(void)
{
    lda8(0xFF);
    wr8(ABS(RIDER_BUMPED), 0xFF);
    wr8(ABS(0x0999), 0xFF);
    player_halt();
}
void PlayerHalt_m1x1(void) { player_halt(); }
void PlayerStopB_m1x1(void) { PlayerStop_m1x1(); }
void PlayerHaltB_m1x1(void) { player_halt(); }
/* $81:AC0B (RTL): an empty hook. */
void EmptyHook_m1x1(void)
{
    retl();
}
/* $80:F98F (RTS): rider X's $01A6 = $0981 = A, then A = its $024E -
 * $023E. */
void SetRiderPair_m1x1(void)
{
    wr8(ABSX(RIDER_POSE), (u8)A);
    wr8(ABSX(0x0981), (u8)A);
    lda8(rd8(ABSX(RIDER_DISTANCE)));
    fc = 1; sbc8(rd8(ABSX(0x023E)));
    ret();
}
/* $80:F8A0 RiderLanePieceAt (RTS): RiderPieceAt in rider X's lane $08A4. */
void RiderLanePieceAt_m1x1(void)
{
    Y = rd8(ABSX(RIDER_LANE_NUM)); nz8((u8)Y);
    RiderPieceAt_m1x1();
}
/* $80:B19D (RTS): for riders 7-0, the piece under the wheel (type $021E,
 * height $0216) and the four piece heights from there on ($0884 + 4X,
 * the column wrapping at $40). */
void ReadRiderPieces_m1x1(void)
{
    ldx8(0x07);
    do {
        lda8((u8)X);
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        wr8(DP(0x0C), (u8)A);
        lda8(0x04); wr8(DP(0x0B), 0x04);
        push16(0xB1AA); RiderLanePieceAt_m1x1();
        wr8(ABSX(RIDER_TILE), (u8)A);
        lda8(rd8(DP(0x10)));
        wr8(ABSX(RIDER_TILE_Y), (u8)A);
        for (;;) {
            push8((u8)A);
            Y = inc8((u8)Y);
            lda8((u8)Y);
            and8(0x3F);
            wr8(DP(0x08), (u8)A);
            Y = rd8(DP(0x0C)); nz8((u8)Y);
            lda8(pull8());
            wr8(ABSY(PIECES_AHEAD), (u8)A);
            wr8(DP(0x0C), inc8(rd8(DP(0x0C))));
            u8 n = dec8(rd8(DP(0x0B)));
            wr8(DP(0x0B), n);
            if (fz) break;
            Y = rd8(DP(0x08)); nz8((u8)Y);
            lda8(rd8((((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF));
        }
        X = dec8((u8)X);
    } while (!fn);
    ret();
}
/* $80:F9DB (RTS, X = rider): the player on the ground (and $84 clear)
 * turns its piece flat ($1F) and sets $B2 from its lane ($80:FA39). */
void FlattenPlayerPiece_m1x1(void)
{
    cmp8((u8)X, 0x00);
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_AIR)));
    ora8(rd8(DP(RACE_END)));
    if (!fz) { ret(); return; }
    player_piece_type(0xF9E8);
    lda8(0x1F);
    wr8(piece_addr(), 0x1F);
    sep(0x10);
    push8((u8)X);
    X = rd8(ABS(RIDER_LANE_NUM)); nz8((u8)X);
    lda8(rd8(ABSX(0xFA06)));
    wr8(DP(COIN_TAKEN), (u8)A);
    sep(0x10);
    X = pull8(); nz8((u8)X);
    ret();
}
/* $80:FA0D (RTS, X = rider): for the player on the ground in a lane with
 * an arrow ($80:FAB0): sound $32, $1979 = 1, and an arrow piece ($15)
 * becomes used ($12), with the course map entry set to the lane's
 * $80:FAA9 and $B7 from $80:FA39. Then StartWheelie. */
void UseArrowPiece_m1x1(void)
{
    cmp8((u8)X, 0x00);
    if (fz) {
        lda8(rd8(ABSX(RIDER_AIR)));
        if (fz) {
            Y = rd8(ABS(RIDER_LANE_NUM)); nz8((u8)Y);
            lda8(rd8(ABSY(0xFA7D)));
            if (!fz) {
                lda8(0x32);
                push8(0x80); push16(0xFA23); QueueSoundB_m1x1();
                lda8(0x01); wr8(ABS(0x1979), 0x01);
                player_piece_type(0xFA2B);
                lda8(rd8(piece_addr()));
                cmp8((u8)A, 0x15);
                if (fz) {
                    lda8(0x12);
                    wr8(piece_addr(), 0x12);
                    push16(X);
                    course_map_slot();
                    lda8(rd8(ABSY(0xFA76)));
                    wr8(LONGX(TRACK_BUF), (u8)A);
                    lda8(rd8(ABSY(0xFA06)));
                    wr8(DP(CLOCK_MINUTE), (u8)A);
                    X = pull16(); nz16(X);
                }
            }
        }
    }
    sep(0x30);
    StartWheelie_m1x1();
}
/* $80:FA84 (RTS, X = rider): the player on the ground collects a pickup:
 * a spark, player $7C's flag $1E8D, and in a lane with one ($80:FB1E)
 * sound $47 and the pickup piece ($16) cleared to flat ($1F), its course
 * map entry set to $80. */
void CollectPickupPiece_m1x1(void)
{
    cmp8((u8)X, 0x00);
    if (fz) {
        lda8(rd8(ABSX(RIDER_AIR)));
        if (fz) {
            push8(0x80); push16(0xFA90); SpawnPlayerSpark_m1x1();
            Y = rd8(DP(COURSE_SLOT)); nz8((u8)Y);
            lda8(0x01); wr8(ABSY(0x1E8D), 0x01);
            Y = rd8(ABS(RIDER_LANE_NUM)); nz8((u8)Y);
            lda8(rd8(ABSY(0xFAEB)));
            if (!fz) {
                lda8(0x47);
                push8(0x80); push16(0xFAA5); QueueSoundB_m1x1();
                player_piece_type(0xFAA8);
                lda8(rd8(piece_addr()));
                cmp8((u8)A, 0x16);
                if (fz) {
                    lda8(0x1F);
                    wr8(piece_addr(), 0x1F);
                    push16(X);
                    course_map_slot();
                    lda8(0x80);
                    wr8(LONGX(TRACK_BUF), 0x80);
                    X = pull16(); nz16(X);
                }
            }
        }
    }
    sep(0x30);
    ret();
}
void HeightBelow_m1x1(void) { height_below(); }
/* $80:F936 (RTS): SetRiderPair(1), then HeightBelow. */
void RiderPairHeight_m1x1(void)
{
    lda8(0x01);
    push16(0xF93A); SetRiderPair_m1x1();
    height_below();
}
/* $80:F947 (RTS, X = rider): the player's speed ($01BE / $01C6) + $40,
 * at most $0340, then RiderPairHeight. */
void PlayerSpeedBoost_m1x1(void)
{
    lda8(rd8(ABS(RIDER_SPEED_LO))); wr8(DP(0x10), (u8)A);
    lda8(rd8(ABS(RIDER_SPEED_HI))); wr8(DP(0x11), (u8)A);
    rep(0x20);
    lda16(rd16(DP(0x10)));
    fc = 0; adc16(0x0040);
    cmp16(A, 0x0340);
    if (fc) lda16(0x0340);
    sep(0x20);
    wr8(ABS(RIDER_SPEED_LO), (u8)A);
    A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
    wr8(ABS(RIDER_SPEED_HI), (u8)A);
    lda8(0x01);
    push16(0xF96E); SetRiderPair_m1x1();
    height_below();
}
/* $80:DD4E Mode0A (RTS): the bonus course: its time and minute checks,
 * the BG tile base, then (once $3A is clear) its step $77 through the
 * table $80:DD5F. */
void Mode0A_m1x1(void)
{
    push16(0xDD50); CheckBonusTime_m1x1();
    push16(0xDD53); EndAtBonusMinute_m1x1();
    push8(0x80); push16(0xDD57); SetBgTileBase_m1x1();
    push8(0x80); push16(0xDD5B); EmptyHook_m1x1();
    lda8(rd8(DP(0x3A)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(MODE_STEP)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    r816_call_pc(0x800000u | rd16(LONGX(0x80DD68)));
}
/* $80:F918 (RTS, X = rider): jump height $01FE and
 * its base $026E = A. */
void SetJumpBase_m1x1(void)
{
    wr8(ABSX(RIDER_HEIGHT), (u8)A);
    wr8(ABSX(RIDER_JUMP_BASE), (u8)A);
    ret();
}
/* $80:F99D (RTS, X = rider): unless it is in jump state 1, SetRiderPair(3)
 * and the height 6 below its base; in state 4 it crash-lands into
 * state 2 ($08AC = 2 too). */
void RiderDropStep_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_JUMP)));
    if (!fz) {
        cmp8((u8)A, 0x01);
        if (fz) { ret(); return; }
    }
    lda8(0x03);
    push16(0xF9AA); SetRiderPair_m1x1();
    lda8(asl8((u8)A));
    push16(0xF9AE); HeightBelow_m1x1();
    lda8(rd8(ABSX(RIDER_JUMP)));
    cmp8((u8)A, 0x04);
    if (!fz) { ret(); return; }
    push16(0xF9B8); CrashLand_m1x1();
    lda8(0x02);
    wr8(ABSX(RIDER_JUMP), 0x02);
    wr8(ABSX(RIDER_LANDED), 0x02);
    ret();
}
/* $80:BF65 (RTS, X = rider): an airborne rider ($01EE) whose height above
 * the ground (less $09C4) has dropped below its fall speed $01B6 lands,
 * unless held ($0904) or not falling ($08AC). The player lands with
 * sound $21 (and in the jump contest, state $C7 = 2 and the landing
 * rank from its pose, $80:C036, if none yet). On landing the jump state
 * clears; then unless crashed ($01CE) or already at speed ($08F4), a
 * landing angle $01E6 outside the window $80:BA73 / $80:BA83 for its
 * speed (+8 when slow) crashes it (sound $3D for the player, $3D,X = $0A,
 * then sound $24 / $29 if on screen); inside it, a changed angle on
 * the flat bounces it ($08CC = 4, $08F4 = 2, $01EE = 1). The fall speed
 * then becomes the height less one. With $84 set it only levels the
 * angle ($01E6 = 6) when still. */
void RiderLanding_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_AIR)));
    if (fz) { ret(); return; }
    push16(0xBF6C); GroundHeightAbove_m1x1();
    sbc8(rd8(ABS(PLAYER_GROUND)));
    wr8(DP(0x01), (u8)A);
    cmp8((u8)A, rd8(ABSX(RIDER_Y)));
    if (fc) { ret(); return; }
    lda8(rd8(ABSX(0x0904)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_LANDED)));
    if (fz) { ret(); return; }
    lda8((u8)X);
    if (fz) {
        lda8(0x21);
        push8(0x80); push16(0xBF89); QueueSoundB_m1x1();
        lda8(rd8(DP(JUMP_STATE)));
        if (!fz) {
            lda8(0x02); wr8(DP(JUMP_STATE), 0x02);
            lda8(rd8(ABS(JUMP_RANK)));
            if (fn) {
                Y = rd8(ABSX(SPRITE_FRAME)); nz8((u8)Y);
                lda8(rd8(ABSY(0xC03F)));
                wr8(ABS(JUMP_RANK), (u8)A);
            }
        }
    }
    lda8(0x00);
    wr8(ABSX(RIDER_AIR), 0);
    wr8(ABSX(RIDER_LANDED), 0);
    lda8(rd8(ABS(PLAYER_GROUND)));
    wr8(ABS(0x09C7), (u8)A);
    lda8(rd8(ABSX(RIDER_BUMPED)));
    if (!fz) goto fall;
    lda8(rd8(DP(RACE_END)));
    if (!fz) {
        lda8(rd8(ABSX(RIDER_POSE)));
        if (!fz) { ret(); return; }
        lda8(0x06); wr8(ABSX(RIDER_LEAN), 0x06);
        ret();
        return;
    }
    Y = rd8(ABSX(RIDER_SLOW));
    wr8(ABSX(RIDER_SLOW), (u8)A);
    lda8((u8)Y);
    if (!fz) goto fall;
    lda8(rd8(ABSX(RIDER_POSE)));
    Y = rd8(ABSX(RIDER_SPEED_HI)); nz8((u8)Y);
    cmp8((u8)Y, 0x02);
    {
        int slow = !fc;
        if (!slow && fz) {
            Y = rd8(ABSX(RIDER_SPEED_LO)); nz8((u8)Y);
            cmp8((u8)Y, 0x80);
            slow = !fc;
        }
        if (slow) { fc = 0; adc8(0x08); }
    }
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSX(RIDER_LEAN)));
    cmp8((u8)A, rd8(ABSY(0xBA7C)));
    if (fc) {
        cmp8((u8)A, rd8(ABSY(0xBA8C)));
        if (!fc) {
            lda8(rd8(ABSX(RIDER_PIECE_LEAN)));
            cmp8((u8)A, rd8(ABSX(RIDER_LEAN)));
            if (!fz) {
                lda8(rd8(ABSX(RIDER_POSE)));
                ora8(rd8(ABSX(RIDER_PIECE_ACTION)));
                if (fz) {
                    lda8(0x04); wr8(ABSX(RIDER_COUNTDOWN), 0x04);
                    lda8(lsr8((u8)A)); wr8(ABSX(RIDER_SLOW), (u8)A);
                    lda8(lsr8((u8)A)); wr8(ABSX(RIDER_AIR), (u8)A);
                }
            }
            goto fall;
        }
    }
    landing_crash();
    return;
fall:
    wr8(DP(0x01), dec8(rd8(DP(0x01))));
    lda8(rd8(DP(0x01)));
    wr8(ABSX(RIDER_Y), (u8)A);
    ret();
}
/* $80:FD4B (RTS): every rider's shadow; an airborne rider near the left
 * or right screen edge (page 0 from $F6, page 1 below $F4) also gets a
 * marker sprite ($16A0 + 4X: X + $0A, the height above $B6, frame
 * $80:F6CC by its $0981, attribute $2F, size 2). */
void DrawShadowsAndMarkers_m1x1(void)
{
    ldx8(0x07);
    do {
        lda8((u8)X);
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        int marker;
        lda8(rd8(ABSX(RIDER_PAGE)));
        cmp8((u8)A, 0x01);
        if (fz) {
            lda8(rd8(ABSX(RIDER_X)));
            cmp8((u8)A, 0xF4);
            marker = !fc;
        } else {
            cmp8((u8)A, 0x00);
            marker = 0;
            if (fz) {
                lda8(rd8(ABSX(RIDER_X)));
                cmp8((u8)A, 0xF6);
                marker = fc;
            }
        }
        if (marker) {
            lda8(rd8(ABSX(RIDER_AIR)));
            if (!fz) {
                lda8(0xB6);
                push16(0xFD75); HeightAbove_m1x1();
                wr8(ABSY(OAM_BUF + 0xA1), (u8)A);
                lda8(rd8(ABSX(RIDER_X)));
                fc = 0; adc8(0x0A);
                wr8(ABSY(OAM_BUF + 0xA0), (u8)A);
                wr8(DP(0x01), (u8)Y);
                Y = rd8(ABSX(0x0981)); nz8((u8)Y);
                lda8(rd8(ABSY(0xF699)));
                Y = rd8(DP(0x01)); nz8((u8)Y);
                wr8(ABSY(OAM_BUF + 0xA2), (u8)A);
                push8((u8)Y);
                lda8((u8)Y);
                fc = 0; adc8(0xA0);
                lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
                Y = (u8)A; nz8((u8)Y);
                lda8(0x02); wr8(ABSY(OAM_SIZE_BUF), 0x02);
                Y = pull8(); nz8((u8)Y);
                lda8(0x2F); wr8(ABSY(OAM_BUF + 0xA3), 0x2F);
            }
        }
        push16(0xFDA6); DrawRiderShadow_m1x1();
        X = dec8((u8)X);
    } while (!fn);
    ret();
}
/* $80:89B2 (RTS): clear $9E / $A2 (words) and $9B, the empty hook, sound
 * 3, then FinishModeSetup (into which it falls). */
void StartModeWithSound_m1x1(void)
{
    rep(0x20);
    wr16(DP(BG1_VSCROLL), 0); wr16(DP(ZOOM), 0);
    sep(0x20);
    wr8(DP(0x9B), 0);
    push8(0x80); push16(0x89BF); EmptyHook_m1x1();
    lda8(0x03);
    push8(0x80); push16(0x89C5); QueueSoundB_m1x1();
    FinishModeSetup_m1x1();
}
/* $80:BCC6 (RTS): while racing ($92), every rider moves, lands and ticks
 * ($08CC); the screen scrolls with the player and the rivals follow; the
 * player's speed high byte $0186 adds into the distance ($0994 fraction,
 * $93), flipping $91 on each carry out. */
void RaceMoveStep_m1x1(void)
{
    lda8(rd8(DP(RACE_STARTED)));
    if (fz) { ret(); return; }
    ldx8(0x07);
    do {
        push16(0xBCCE); AddSpeedToX_m1x1();
        push16(0xBCD1); RiderLanding_m1x1();
        push16(0xBCD4); RiderLandCheck_m1x1();
        push16(0xBCD7); Tick08CC_m1x1();
        X = dec8((u8)X);
    } while (!fn);
    push16(0xBCDD); ScrollByPlayerSpeed_m1x1();
    push16(0xBCE0); ScrollRivalsRelative_m1x1();
    lda8(rd8(ABS(RIDER_STEP)));
    lda8(lsr8((u8)A)); lda8(ror8((u8)A)); lda8(ror8((u8)A)); lda8(ror8((u8)A));
    fc = 0; adc8(rd8(ABS(SCROLL_FRAC)));
    wr8(ABS(SCROLL_FRAC), (u8)A);
    lda8(rd8(ABS(RIDER_STEP)));
    adc8(rd8(DP(0x93)));
    wr8(DP(0x93), (u8)A);
    if (fc) {
        lda8(rd8(DP(0x91)));
        eor8(0x01);
        wr8(DP(0x91), (u8)A);
    }
    ret();
}
/* $80:BD12 (RTL): ScrollByPlayerSpeed for other banks. */
void ScrollByPlayerSpeedLong_m1x1(void)
{
    push16(0xBD14); ScrollByPlayerSpeed_m1x1();
    retl();
}
/* $80:CE95 (RTL): PlaceRiderShadow with DB = $80. */
void PlaceRiderShadowLong_m1x1(void)
{
    push8(DB);
    lda8(0x80); push8(0x80);
    DB = pull8(); nz8(DB);
    push16(0xCE9C); PlaceRiderShadow_m1x1();
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:FE31 (RTL): AddScore for other banks. */
void AddScoreLong_m1x1(void)
{
    push16(0xFE33); AddScore_m1x1();
    retl();
}
/* $80:F972 (RTS, X = rider): SetRiderPair(4), the jump base at 8 (+$10
 * more when $0206 is set), and $0246 = $40. */
void RiderJumpBase_m1x1(void)
{
    lda8(0x04);
    push16(0xF976); SetRiderPair_m1x1();
    lda8(asl8((u8)A));
    push16(0xF97A); SetJumpBase_m1x1();
    lda8(rd8(ABSX(RIDER_LANDING)));
    if (!fz) {
        lda8(rd8(ABSX(RIDER_HEIGHT)));
        fc = 0; adc8(0x10);
        push16(0xF988); SetJumpBase_m1x1();
    }
    lda8(0x40);
    wr8(ABSX(RIDER_SLOW_AT), 0x40);
    ret();
}
/* $81:C5D7 (RTS, Y = slot): the shadow of the rider in slot Y ($0950),
 * with $07 from $81:C5CF; a rider in the middle of screen page 1 ($70-
 * $8F) sets $092A. */
void SlotRiderShadow_m1x1(void)
{
    wr8(DP(0x05), (u8)Y);
    lda8(rd8(ABSY(RANK_ORDER)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSY(0xC5CF)));
    wr8(DP(0x07), (u8)A);
    lda8(rd8(ABSX(RIDER_PAGE)));
    cmp8((u8)A, 0x01);
    if (fz) {
        lda8(rd8(ABSX(RIDER_X)));
        cmp8((u8)A, 0x70);
        if (fc) {
            cmp8((u8)A, 0x90);
            if (!fc) { lda8(0x01); wr8(ABS(0x092A), 0x01); }
        }
    }
    push8(0x81); push16(0xC5FC); PlaceRiderShadowLong_m1x1();
    ret();
}
/* $82:9159 (RTL): the stadium bonus for $0928 (3 bytes at $82:9131 + 4 *
 * (stadium - 2)) into $09AD-$09AF and added to the score; $00 = stadium
 * - 2. X is kept. */
void AddStadiumBonus_m1x1(void)
{
    push8(DB);
    lda8(0x82); push8(0x82);
    DB = pull8(); nz8(DB);
    push8((u8)X);
    lda8(rd8(ABS(LAP)));
    lda8(dec8((u8)A)); lda8(dec8((u8)A));
    wr8(DP(0x00), (u8)A);
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0x9131))); wr8(ABS(SCORE_ADD + 0x01), (u8)A);
    lda8(rd8(ABSX(0x9132))); wr8(ABS(SCORE_ADD + 0x02), (u8)A);
    lda8(rd8(ABSX(0x9133))); wr8(ABS(SCORE_ADD + 0x03), (u8)A);
    push8(0x82); push16(0x917E); AddScoreLong_m1x1();
    X = pull8(); nz8((u8)X);
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:B0F3 (RTS, X = rider): look at the four pieces ahead ($0884 + 4X).
 * A hurdle ($C0/$C1 or $70-$73) sends the rider looking for a flat lane
 * at that column ($40 rows), up first (lanes above, from $08A4 - 1 to
 * 1) and then down (to 5), steering into the first one found; a ramp
 * top ($48-$4A) sets the target angle $0971 = 9; otherwise $0971 = the
 * piece angle $08B4 (0 while landing, $0176). Returns A = the hurdle
 * count $0D. */
void CpuAvoidHurdle_m1x1(void)
{
    lda8(0x00);
    wr8(DP(0x09), 0); wr8(DP(0x0D), 0);
    lda8((u8)X);
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    wr8(DP(0x0A), (u8)A);
    ldy8(0x03);
    for (;;) {
        wr8(DP(0x0B), (u8)Y);
        Y = rd8(DP(0x0A)); nz8((u8)Y);
        lda8(rd8(ABSY(PIECES_AHEAD)));
        int hurdle;
        cmp8((u8)A, 0xC0); hurdle = fz;
        if (!hurdle) { cmp8((u8)A, 0xC1); hurdle = fz; }
        if (!hurdle) {
            cmp8((u8)A, 0x70);
            if (fc) { cmp8((u8)A, 0x74); hurdle = !fc; }
        }
        if (hurdle) goto avoid;
        cmp8((u8)A, 0x48);
        if (fc) {
            cmp8((u8)A, 0x4B);
            if (!fc) { lda8(0x09); goto store; }
        }
        wr8(DP(0x0A), inc8(rd8(DP(0x0A))));
        Y = rd8(DP(0x0B)); nz8((u8)Y);
        Y = dec8((u8)Y);
        if (fn) goto angle;
    }
avoid:
    wr8(DP(0x0D), inc8(rd8(DP(0x0D))));
    lda8(rd8(DP(0x0A)));
    and8(0x03);
    Y = (u8)A; nz8((u8)Y);
    Y = inc8((u8)Y);
    lda8((u8)Y);
    fc = 0; adc8(rd8(ABSX(RIDER_COLUMN)));
    and8(0x3F);
    wr8(DP(0x08), (u8)A);
    lda8(0x40); wr8(DP(0x09), 0x40);
    Y = rd8(ABSX(RIDER_LANE_NUM)); nz8((u8)Y);
    for (;;) {
        Y = dec8((u8)Y);
        cmp8((u8)Y, 0x01);
        if (fn) break;
        push16(0xB153); LanePieceAt_m1x1();
        Y = rd8(DP(0x07)); nz8((u8)Y);
        push16(0xB158); IsPieceFlat_m1x1();
        if (!fc) { push16(0xB15D); CpuSteerUp_m1x1(); goto angle; }
    }
    Y = rd8(ABSX(RIDER_LANE_NUM)); nz8((u8)Y);
    for (;;) {
        Y = inc8((u8)Y);
        cmp8((u8)Y, 0x06);
        if (!fn) goto angle;
        push16(0xB16B); LanePieceAt_m1x1();
        Y = rd8(DP(0x07)); nz8((u8)Y);
        push16(0xB170); IsPieceFlat_m1x1();
        if (!fc) { push16(0xB175); CpuSteerDown_m1x1(); goto angle; }
    }
angle:
    ldy8(0x00);
    lda8(rd8(ABSX(RIDER_SCRIPT)));
    if (fz) { Y = rd8(ABSX(RIDER_PIECE_LEAN)); nz8((u8)Y); }
    lda8((u8)Y);
store:
    wr8(ABSX(RIDER_WANT_LEAN), (u8)A);
    lda8(rd8(DP(0x0D)));
    ret();
}
/* $80:BDE5 (RTS, X = rival): place a rival entering the screen: a random
 * lane ($80:BAF2), X $A0 on page 0 or $40 on page 1, angle 6, the
 * player's column + $80:BB00 and $018E. Entering on page 1 it must not
 * land on a piece taller than $3B-$92 (then A = 0 and nothing more);
 * on a $E6 ramp it starts mid-jump (state 1 at height $30 from lane $20
 * down, else state 3). Then its target speed (RivalTarget), a 1-in-64
 * chance of a fast start ($01C6 = (page ^ 2) * 2), $3B = $10 and
 * $01DE = 1. */
void PlaceRival_m1x1(void)
{
    push8(0x80); push16(0xBDE8); RandomB_m1x1();
    and8(0x03);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xBAFB)));
    wr8(ABSX(RIDER_LANE), (u8)A);
    Y = rd8(ABSX(RIDER_PAGE)); nz8((u8)Y);
    lda8(fz ? 0xA0 : 0x40);
    wr8(ABSX(RIDER_X), (u8)A);
    lda8(0x06); wr8(ABSX(RIDER_LEAN), 0x06);
    lda8(rd8(ABS(RIDER_COLUMN)));
    fc = 0; adc8(rd8(ABSY(0xBB09)));
    and8(0x3F);
    wr8(ABSX(RIDER_COLUMN), (u8)A);
    lda8(rd8(ABS(RIDER_WHEEL)));
    wr8(ABSX(RIDER_WHEEL), (u8)A);
    lda8((u8)Y);
    if (!fz) {
        ldy8(0x05);
        push16(0xBE1E); RiderPieceAt_m1x1();
        lda8(rd8(DP(0x10)));
        cmp8((u8)A, 0xE4);
        if (!fz) {
            cmp8((u8)A, 0x3B);
            if (!fc) { lda8(0x00); ret(); return; }
            cmp8((u8)A, 0x93);
            if (fc) { lda8(0x00); ret(); return; }
            ldy8(0x02);
            push16(0xBE31); RiderPieceAt_m1x1();
            cmp8((u8)A, 0xE6);
            if (fz) {
                lda8(rd8(ABSX(RIDER_LANE)));
                cmp8((u8)A, 0x20);
                if (fc) {
                    lda8(0x30); wr8(ABSX(RIDER_HEIGHT), 0x30);
                    lda8(0x01);
                } else lda8(0x03);
                wr8(ABSX(RIDER_JUMP), (u8)A);
            }
        }
    }
    Y = rd8(ABSX(RIDER_PAGE)); nz8((u8)Y);
    push16(0xBE50); RivalTarget_m1x1();
    push8(0x80); push16(0xBE54); RandomB_m1x1();
    and8(0x3F);
    if (fz) {
        lda8(rd8(ABSX(RIDER_PAGE)));
        eor8(0x02);
        lda8(asl8((u8)A));
        wr8(ABSX(RIDER_SPEED_HI), (u8)A);
    }
    lda8(0x10); wr8(DP(0x3B), 0x10);
    lda8(0x01); wr8(ABSX(RIDER_ACTIVE), 0x01);
    ret();
}
/* $80:AF48 (RTS): the CPU riders 1-7 on the track ($01DE) and not crashed,
 * off the track or changing lanes: steer; on the ground also throttle,
 * look for hurdles, and with none the lane tile check, retargeting (when
 * $092A) and (unless $84) a lane decision. In the air the target angle
 * $0971 follows the piece angle $08B4. */
void CpuRidersStep_m1x1(void)
{
    ldx8(0x01);
    do {
        wr8(DP(0x0F), (u8)X);
        lda8(rd8(ABSX(RIDER_ACTIVE)));
        if (fz) goto next;
        lda8(rd8(ABSX(RIDER_BUMPED)));
        ora8(rd8(ABSX(RIDER_TIMER)));
        ora8(rd8(ABSX(0x199E)));
        if (!fz) goto next;
        push16(0xAF5E); CpuSteer_m1x1();
        lda8(rd8(ABSX(RIDER_AIR)));
        if (!fz) {
            lda8(rd8(ABSX(RIDER_PIECE_LEAN)));
            wr8(ABSX(RIDER_WANT_LEAN), (u8)A);
            goto next;
        }
        push16(0xAF66); CpuThrottle_m1x1();
        push16(0xAF69); CpuAvoidHurdle_m1x1();
        if (!fz) goto next;
        push8(0x80); push16(0xAF6F); CheckLaneTile_m1x1();
        lda8(rd8(ABS(0x092A)));
        if (fz) goto next;
        push16(0xAF77); RetargetRival_m1x1();
        lda8(rd8(DP(RACE_END)));
        if (!fz) goto next;
        push16(0xAF7E); CpuLaneDecision_m1x1();
    next:
        X = rd8(DP(0x0F)); nz8((u8)X);
        X = inc8((u8)X);
        cmp8((u8)X, 0x08);
    } while (!fc);
    ret();
}
/* $80:BD58 (RTS): distances, then every rival slot 7-1 not on the track
 * whose distance $085E is ahead of the player's $9C by $170 or more, or
 * behind by up to $A0 (when the player is that far in), gets placed
 * (PlaceRival), unless $84. */
void SpawnRivals_m1x1(void)
{
    push16(0xBD5A); UpdateRiderDistances_m1x1();
    ldy8(0x0E);
    ldx8(0x07);
    do {
        lda8(rd8(ABSX(RIDER_ACTIVE)));
        if (fz) {
            rep(0x20);
            lda16(rd16(DP(SCROLL_X)));
            fc = 0; adc16(0x0170);
            cmp16(A, rd16(ABSY(RIDER_DIST)));
            int place = fc;
            if (!place) {
                lda16(rd16(DP(SCROLL_X)));
                fc = 1; sbc16(0x00A0);
                if (!fn) {
                    cmp16(A, rd16(ABSY(RIDER_DIST)));
                    place = fc;
                }
            }
            if (place) {
                sep(0x20);
                lda8(rd8(DP(RACE_END)));
                if (fz) {
                    push8((u8)Y);
                    push16(0xBD87); PlaceRival_m1x1();
                    Y = pull8(); nz8((u8)Y);
                }
            }
        }
        sep(0x20);
        X = dec8((u8)X);
        Y = dec8((u8)Y);
        Y = dec8((u8)Y);
    } while (!fz);
    ret();
}
/* $80:FDAB (RTS, X = rider): the player crossing the finish once ($03AC,
 * not $84): $5E = $AC = $1D; with a finish result $093F sound $4A (1st)
 * or $28, the stadium bonus, $D4 = $80, a win counted ($1E87) without
 * rivals, and $5E cleared when $60; with none, $84 = 1 and sounds $2B,
 * $2D, $80, $5E cleared. */
void CrossFinish_m1x1(void)
{
    cmp8((u8)X, 0x00);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(FINISH_CROSSED)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(RACE_END)));
    if (!fz) { ret(); return; }
    wr8(ABS(FINISH_CROSSED), inc8(rd8(ABS(FINISH_CROSSED))));
    lda8(0x1D); wr8(DP(0x5E), 0x1D); wr8(DP(0xAC), 0x1D);
    lda8(rd8(ABS(LAPS_LEFT)));
    if (!fz) {
        cmp8((u8)A, 0x01);
        if (fz) { lda8(0x4A); push8(0x80); push16(0xFDCF); QueueSoundB_m1x1(); }
        else { lda8(0x28); push8(0x80); push16(0xFDD7); QueueSoundB_m1x1(); }
        push8(0x80); push16(0xFDDB); AddStadiumBonus_m1x1();
        lda8(0x80); wr8(DP(0xD4), 0x80);
        lda8(rd8(DP(FINAL_RACE)));
        if (fz) wr8(ABS(0x1E87), inc8(rd8(ABS(0x1E87))));
        lda8(rd8(DP(OVERHEATED)));
        if (fz) { ret(); return; }
    } else {
        lda8(0x01); wr8(DP(RACE_END), 0x01);
        lda8(0x2B); push8(0x80); push16(0xFDF5); QueueSoundB_m1x1();
        lda8(0x2D); push8(0x80); push16(0xFDFB); QueueSoundB_m1x1();
        lda8(0x80); push8(0x80); push16(0xFE01); QueueSoundB_m1x1();
    }
    lda8(0x00); wr8(DP(0x5E), 0x00);
    ret();
}
/* $82:84B2 (RTL): A = the byte at $82:84BA + A. X is kept. */
void ByteTable84BA_m1x1(void)
{
    push8((u8)X);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(LONGX(0x8284BA)));
    X = pull8(); nz8((u8)X);
    retl();
}
/* $80:BF0C (RTS): when the ball is due ($092C) and rider 7 is off screen
 * (not page 1, page 0 left of $EA), it enters as rider 3 on page 2 via
 * PlaceRival, with $01C9 = 0, $01B9 = $08C7 = $92, and crashes into its
 * landing state. */
void LaunchBall_m1x1(void)
{
    lda8(rd8(ABS(0x092C)));
    if (fz) { ret(); return; }
    lda8(rd8(ABS(RIDER_ACTIVE + 0x03)));
    ldx8(0x07);
    lda8(rd8(ABSX(RIDER_PAGE)));
    lda8(lsr8((u8)A));
    if (fc) { ret(); return; }
    lda8(lsr8((u8)A));
    if (!fc) {
        lda8(rd8(ABSX(RIDER_X)));
        cmp8((u8)A, 0xEA);
        if (fc) { ret(); return; }
    }
    ldy8(0x00);
    wr8(ABS(0x092C), 0);
    Y = inc8((u8)Y);
    ldx8(0x02);
    wr8(ABS(RIDER_PAGE + 0x03), 0x02);
    X = inc8((u8)X);
    push16(0xBF34); PlaceRival_m1x1();
    if (fz) { ret(); return; }
    lda8(0x00); wr8(ABS(RIDER_SPEED_HI + 0x03), 0x00);
    lda8(0x92); wr8(ABS(RIDER_Y + 0x03), 0x92); wr8(ABS(BALL_OUT), 0x92);
    landing_crash();
}
/* $80:D172 (RTL): StepObjectX for object X + 8. */
void StepObjectX8_m1x1(void)
{
    lda8((u8)X);
    fc = 0; adc8(0x08);
    X = (u8)A; nz8((u8)X);
    StepObjectX_m1x1();
}
/* $80:D14E (RTL, X = object): move object X and X + 8, count up its timer
 * $1518 and draw its marker; once the timer turns positive, clear $1530
 * and count $1541 up to 5. */
void MarkerObjectStep_m1x1(void)
{
    push8(0x80); push16(0xD151); StepObjectX_m1x1();
    push8((u8)X);
    push8(0x80); push16(0xD156); StepObjectX8_m1x1();
    X = pull8(); nz8((u8)X);
    wr8(ABSX(MARKER_VY), inc8(rd8(ABSX(MARKER_VY))));
    push8(0x80); push16(0xD15E); DrawRiderMarker_m1x1();
    lda8(rd8(ABSX(MARKER_VY)));
    if (!fn) {
        wr8(ABSX(MARKER_ON), 0);
        lda8(rd8(ABS(MARKER_COUNT)));
        cmp8((u8)A, 0x05);
        if (!fc) wr8(ABS(MARKER_COUNT), inc8(rd8(ABS(MARKER_COUNT))));
    }
    retl();
}
/* $80:FECD (RTS): QueueBonusDigits at VRAM $1B38, size $20. */
void QueueBonusDigitsAt_m1x1(void)
{
    lda8(0x38); wr8(DP(0x00), 0x38);
    lda8(0x1B); wr8(DP(0x01), 0x1B);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    QueueBonusDigits_m1x1();
}
/* $80:D134 (RTL): the coin rank, then each active marker object (even
 * slots 6-0, $1530) counts its word timer $1538 up and steps. */
void MarkerObjects_m1x1(void)
{
    push16(0xD136); ShowCoinRank_m1x1();
    ldx8(0x06);
    do {
        lda8(rd8(ABSX(MARKER_ON)));
        if (!fz) {
            rep(0x20);
            wr16(ABSX(MARKER_AGE), inc16(rd16(ABSX(MARKER_AGE))));
            sep(0x20);
            push8(0x80); push16(0xD148); MarkerObjectStep_m1x1();
        }
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
    retl();
}
/* $80:B211 (RTS): $7F = frame & 7, the riders' pieces, and in the race
 * modes (below 9, no fade $1923, not $0CD7) the pace, score and bonus
 * digits. */
void RaceFrameStep_m1x1(void)
{
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x07);
    wr8(DP(RIDER_TURN), (u8)A);
    push16(0xB219); ReadRiderPieces_m1x1();
    lda8(rd8(ABS(ATTRACT_DEMO)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(FADE_STATE)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(GAME_MODE)));
    cmp8((u8)A, 0x09);
    if (fc) { ret(); return; }
    push16(0xB22C); TrackRiderPace_m1x1();
    push16(0xB22F); AddScore_m1x1();
    push16(0xB232); QueueBonusDigitsAt_m1x1();
    ret();
}
/* $82:8424 (RTL): the five digits at [$18] (leading zeros as $0A) and a
 * trailing 0 through the font table, +$10, queued from $03CF (size $16)
 * at VRAM $00. X and Y are kept. */
void QueueFontDigits_m1x1(void)
{
    push8((u8)X); push8((u8)Y);
    ldx8(0x00); ldy8(0x00);
    do {
        lda8(rd8((ILDP(0x18) + Y) & 0xFFFFFF));
        int blank = 0;
        if (fz) { cmp8((u8)X, 0x00); blank = fz; }
        if (blank) lda8(0x0A);
        else X = inc8((u8)X);
        push8(0x82); push16(0x843A); ByteTable84BA_m1x1();
        fc = 0; adc8(0x10);
        wr8(ABSY(DIGIT_ROW), (u8)A);
        Y = inc8((u8)Y);
        cmp8((u8)Y, 0x05);
    } while (!fc);
    lda8(0x00);
    push8(0x82); push16(0x844B); ByteTable84BA_m1x1();
    fc = 0; adc8(0x10);
    wr8(ABS(DIGIT_ROW + 0x05), (u8)A);
    lda8(0x16); wr8(ABS(DIGIT_BUF), 0x16);
    rep(0x20);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x10); wr8(DP(0x0F), 0x10);
    push8(0x82); push16(0x8467); VramQueueTilesLong_m1x1();
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    retl();
}
/* $81:9828 (RTS): an empty step. */
void EmptyStep9828_m1x1(void)
{
    ret();
}
/* $81:9788 SpawnObject (RTL, Y = type, $00 / $02 = X / Y position): take
 * the first free object slot (even 14-0, $02EF): type $02FF, active,
 * script $81:AD67, position, and its timers, frames and velocities
 * cleared; then the type's init routine ($81:AD7B, $0E = slot). C set
 * when no slot is free. X is kept. */
void SpawnObject_m1x1(void)
{
    push8(DB);
    lda8(0x81); push8(0x81);
    DB = pull8(); nz8(DB);
    push8((u8)X);
    ldx8(0x0E);
    for (;;) {
        lda8(rd8(ABSX(OBJ_ACTIVE)));
        if (fz) break;
        X = dec8((u8)X);
        X = dec8((u8)X);
        if (fn) {
            X = pull8(); nz8((u8)X);
            DB = pull8(); nz8(DB);
            fc = 1;
            retl();
            return;
        }
    }
    lda8((u8)Y);
    wr8(ABSX(OBJ_TYPE), (u8)A);
    lda8(0x01); wr8(ABSX(OBJ_ACTIVE), 0x01);
    rep(0x20);
    lda16(Y);
    lda16(asl16(A));
    Y = (u8)A; nz8((u8)Y);
    lda16(rd16(ABSY(0xAD67))); wr16(ABSX(OBJ_SCRIPT), A);
    lda16(rd16(DP(0x00))); wr16(ABSX(OBJ_X), A);
    lda16(rd16(DP(0x02))); wr16(ABSX(OBJ_Y), A);
    static const u16 clr[11] = { 0x031F, 0x032F, 0x036F, 0x037F, 0x030F, 0x035F,
                                 0x034F, 0x029F, 0x02AF, 0x02BF, 0x02CF };
    for (int i = 0; i < 11; i++) wr16(ABSX(clr[i]), 0);
    sep(0x20);
    wr8(DP(0x0E), (u8)X);
    X = Y; nz8((u8)X);
    push16(0x97E3);
    r816_call_pc(0x810000u | rd16(LONGX(0x81AD7B)));
    X = pull8(); nz8((u8)X);
    DB = pull8(); nz8(DB);
    fc = 0;
    retl();
}
/* $81:9829 FindObjectSlot (RTL, A = type): Y = the slot (even 14-0) of
 * the object of type A, or negative when none. */
void FindObjectSlot_m1x1(void)
{
    ldy8(0x0E);
    do {
        cmp8((u8)A, rd8(ABSY(OBJ_TYPE)));
        if (fz) break;
        Y = dec8((u8)Y);
        Y = dec8((u8)Y);
    } while (!fn);
    retl();
}
/* $82:8259 (RTL): the three bytes at [$18] (minutes, seconds, hundredths)
 * as a time "M'SS"HH" in two rows ($03D0-$03D7, lower row +$10; a
 * leading zero minute tens blank $0C, the marks $0A / $0B), size $28,
 * queued at VRAM $00. X and Y are kept. */
void QueueTimeDigits_m1x1(void)
{
    push8((u8)X); push8((u8)Y);
    ldx8(0x00); ldy8(0x00);
    lda8(rd8((ILDP(0x18) + Y) & 0xFFFFFF));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        X = inc8((u8)X);
    }
    wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x09), (u8)A);
    lda8((u8)X);
    cmp8((u8)A, 0x00);
    if (fz) lda8(0x0C);
    wr8(ABS(DIGIT_ROW), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x08), (u8)A);
    static const u16 lo[2] = { 0x03D4, 0x03D7 };
    for (int i = 0; i < 2; i++) {
        Y = inc8((u8)Y);
        ldx8(0x00);
        lda8(rd8((ILDP(0x18) + Y) & 0xFFFFFF));
        for (;;) {
            cmp8((u8)A, 0x0A);
            if (!fc) break;
            sbc8(0x0A);
            X = inc8((u8)X);
        }
        wr8(ABS(lo[i]), (u8)A);
        ora8(0x10); wr8(ABS(lo[i] + 8), (u8)A);
        lda8((u8)X);
        wr8(ABS(lo[i] - 1), (u8)A);
        ora8(0x10); wr8(ABS(lo[i] + 7), (u8)A);
    }
    lda8(0x0A); wr8(ABS(DIGIT_ROW + 0x02), 0x0A);
    lda8(inc8((u8)A)); wr8(ABS(DIGIT_ROW + 0x05), (u8)A);
    lda8(0x1A); wr8(ABS(DIGIT_ROW + 0x0A), 0x1A);
    lda8(inc8((u8)A)); wr8(ABS(DIGIT_ROW + 0x0D), (u8)A);
    lda8(0x28); wr8(ABS(DIGIT_BUF), 0x28);
    rep(0x20);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x01); wr8(DP(0x0F), 0x01);
    push8(0x82); push16(0x82E6); VramQueueTilesLong_m1x1();
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    retl();
}
/* $80:98DC (RTS): the results list, one line every $20 frames ($88): the
 * time of entry $044F (3 bytes at $083D) at its VRAM row ($80:993F),
 * sound $42; after five, the next end-screen steps ($0450 = 4 and
 * $0451 = 1, or 2 when $085C is set). */
void EndScreenTimes_m1x1(void)
{
    u8 t = dec8(rd8(DP(MODE_TIMER)));
    wr8(DP(MODE_TIMER), t);
    if (!fz) { ret(); return; }
    lda8(0x20); wr8(DP(MODE_TIMER), 0x20);
    lda8(rd8(ABS(ENTRY_CURSOR)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    fc = 0; adc8(rd8(ABS(ENTRY_CURSOR)));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0x083D))); wr8(ABS(RACE_MIN), (u8)A);
    lda8(rd8(ABSY(0x083E))); wr8(ABS(RACE_SEC), (u8)A);
    lda8(rd8(ABSY(0x083F))); wr8(ABS(RACE_FRAMES), (u8)A);
    rep(0x20);
    lda16(rd16(ABSX(0x993D))); wr16(DP(0x00), A);
    lda16(0x03AE); wr16(DP(0x18), 0x03AE);
    sep(0x20);
    lda8(0x00); wr8(DP(0x1A), 0x00);
    push8(0x80); push16(0x9915); QueueTimeDigits_m1x1();
    lda8(0x42);
    push8(0x80); push16(0x991B); QueueSoundB_m1x1();
    wr8(ABS(ENTRY_CURSOR), inc8(rd8(ABS(ENTRY_CURSOR))));
    lda8(rd8(ABS(ENTRY_CURSOR)));
    cmp8((u8)A, 0x05);
    if (!fc) { ret(); return; }
    lda8(rd8(ABS(0x085C)));
    if (fz) {
        lda8(0x04); wr8(ABS(END_STEP), 0x04);
        lda8(0x01); wr8(ABS(0x0451), 0x01);
    } else {
        lda8(0x02); wr8(ABS(END_STEP), 0x02);
    }
    ret();
}
void FrameSequenceA_m1x1(void) { frame_sequence(0x04, 0xC081, 0x02, 0x001C, 0xC07D, 0xC055); }
void FrameSequenceB_m1x1(void) { frame_sequence(0x06, 0xC0F0, 0x03, 0x0018, 0xC0EA, 0xC0C2); }
/* $81:B96C (RTS): object $03A3 drifts every 4 frames, its X by the word
 * $031F (reversing every 8 steps, $032F) and its Y up; above row $48 it
 * parks at ($F0, $F0), steps its state, removes the type-9 and type-0
 * objects and waits $80-$FF frames. */
void DriftUpObject_m1x1(void)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x03);
    if (fz) {
        rep(0x20);
        lda16(rd16(ABSX(OBJ_STEP)));
        fc = 0; adc16(rd16(ABSX(OBJ_X)));
        wr16(ABSX(OBJ_X), A);
        wr16(ABSX(OBJ_RANDOM), dec16(rd16(ABSX(OBJ_RANDOM))));
        if (fz) {
            lda16(0x0008); wr16(ABSX(OBJ_RANDOM), 0x0008);
            lda16(rd16(ABSX(OBJ_STEP)));
            eor16(0xFFFF);
            lda16(inc16(A));
            wr16(ABSX(OBJ_STEP), A);
        }
    }
    sep(0x20);
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x03);
    if (!fz) { ret(); return; }
    wr8(ABSX(OBJ_Y), dec8(rd8(ABSX(OBJ_Y))));
    lda8(rd8(ABSX(OBJ_Y)));
    cmp8((u8)A, 0x48);
    if (fc) { ret(); return; }
    rep(0x20);
    lda16(0x00F0);
    wr16(ABSX(OBJ_X), 0x00F0);
    wr16(ABSX(OBJ_Y), 0x00F0);
    sep(0x20);
    wr8(ABSX(OBJ_STATE), inc8(rd8(ABSX(OBJ_STATE))));
    lda8(0x09);
    push8(0x81); push16(0xB9BD); FindObjectSlot_m1x1();
    lda8(0x00); wr8(ABSY(OBJ_ACTIVE), 0x00);
    lda8(0x00);
    push8(0x81); push16(0xB9C8); FindObjectSlot_m1x1();
    lda8(0x00); wr8(ABSY(OBJ_ACTIVE), 0x00);
    push8(0x81); push16(0xB9D1); RandomB_m1x1();
    and8(0x7F);
    fc = 0; adc8(0x80);
    wr8(ABSX(OBJ_DELAY), (u8)A);
    ret();
}
/* $81:B9DB (RTS): object $03A3 releases balloons: sound $2B once at the
 * start (before place 4, $086E), then on odd-high frames of the counter
 * $8A whenever its timer $036F is out: count ($033F), wait $20-$5F
 * frames and spawn a type-9 object at X $40-$BF, Y $D0 with sound $48
 * and a random rise $B6-$C5 ($02AF). From place 4 on, only until the
 * first one. */
void ReleaseBalloons_m1x1(void)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(ABS(PLAYER_PLACE)));
    cmp8((u8)A, 0x04);
    if (fc) {
        lda8(rd8(ABSX(0x033F)));
        if (!fz) { ret(); return; }
    } else {
        lda8(rd8(DP(TIMER_A)));
        if (fz) {
            lda8(rd8(ABSX(0x033F)));
            if (fz) {
                lda8(0x2B);
                push8(0x81); push16(0xB9FA); QueueSoundB_m1x1();
            }
        }
    }
    rep(0x20);
    wr16(DP(TIMER_A), inc16(rd16(DP(TIMER_A))));
    sep(0x20);
    lda8(rd8(DP(0x8B)));
    and8(0x01);
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(OBJ_DELAY)));
    if (!fz) { ret(); return; }
    wr8(ABSX(0x033F), inc8(rd8(ABSX(0x033F))));
    push8(0x81); push16(0xBA12); RandomB_m1x1();
    and8(0x3F);
    fc = 0; adc8(0x20);
    wr8(ABSX(OBJ_DELAY), (u8)A);
    ldy8(0x09);
    push8(0x81); push16(0xBA20); SpawnObject_m1x1();
    if (fc) { ret(); return; }
    X = rd8(DP(0x0E)); nz8((u8)X);
    push8(0x81); push16(0xBA28); RandomB_m1x1();
    and8(0x7F);
    fc = 0; adc8(0x40);
    wr8(ABSX(OBJ_X), (u8)A);
    lda8(0xD0); wr8(ABSX(OBJ_Y), 0xD0);
    lda8(0x48);
    push8(0x81); push16(0xBA3B); QueueSoundB_m1x1();
    push8(0x81); push16(0xBA3F); RandomB_m1x1();
    and8(0x0F);
    fc = 0; adc8(0xB6);
    wr8(ABSX(OBJ_VY), (u8)A);
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    ret();
}
/* $80:997C (RTS): the end screen's record entry. After the $88 delay the
 * box (2 rows of $20 from $7E:5800) is queued at $0300; then a press of
 * A ($26 bit 7) gives step $0450 = 1, $88 = $20, the next box ($7E:5980)
 * and the record time at $11:572C at VRAM $09D3 with sound $43. */
void EndScreenRecord_m1x1(void)
{
    lda8(rd8(DP(MODE_TIMER)));
    if (!fz) {
        u8 t = dec8(rd8(DP(MODE_TIMER)));
        wr8(DP(MODE_TIMER), t);
        if (!fz) { ret(); return; }
        lda8(0x00); wr8(DP(0x02), 0x00);
        lda8(0x58); wr8(DP(0x03), 0x58);
        lda8(0x7E); wr8(DP(0x04), 0x7E);
        lda8(0x20); wr8(DP(0x10), 0x20);
        lda8(0x02); wr8(DP(0x11), 0x02);
        lda8(0x00); wr8(DP(0x00), 0x00);
        lda8(0x03); wr8(DP(0x01), 0x03);
        push16(0x99A2); QueueTileRows_m1x1();
    }
    lda8(rd8(DP(P1_PRESS_LO)));
    and8(0x80);
    if (fz) { ret(); return; }
    lda8(0x01); wr8(ABS(END_STEP), 0x01);
    lda8(0x20); wr8(DP(MODE_TIMER), 0x20);
    lda8(0x80); wr8(DP(0x02), 0x80);
    lda8(0x59); wr8(DP(0x03), 0x59);
    lda8(0x7E); wr8(DP(0x04), 0x7E);
    lda8(0x20); wr8(DP(0x10), 0x20);
    lda8(0x02); wr8(DP(0x11), 0x02);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x03); wr8(DP(0x01), 0x03);
    push16(0x99D0); QueueTileRows_m1x1();
    rep(0x20);
    lda16(0x09D3); wr16(DP(0x00), 0x09D3);
    lda16(0x572C); wr16(DP(0x18), 0x572C);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x99E6); QueueTimeDigits_m1x1();
    lda8(0x43);
    push8(0x80); push16(0x99EC); QueueSoundB_m1x1();
    ret();
}
/* $81:973F (RTS): MoveObjectX for object X + 8. */
void MoveObjectX8_m1x1(void)
{
    lda8((u8)X);
    fc = 0; adc8(0x08);
    X = (u8)A; nz8((u8)X);
    MoveObjectX_m1x1();
}
/* $81:9604 (RTS, Y = object): a falling confetti piece: its frame $011E
 * cycles every 4 frames ($0126); it moves (X and X + 8), its sprite
 * ($1610 + 2Y: position $0106 / $010E, tile and attribute from
 * $81:966F / $81:9673 by frame, frame 0 from timer $F0 on, size 2),
 * and when its timer $0146 wraps it goes to state 3 with a $20 wait. */
void ConfettiStep_m1x1(void)
{
    X = Y; nz8((u8)X);
    wr8(ABSX(EFFECT_TIMER), dec8(rd8(ABSX(EFFECT_TIMER))));
    if (fz) {
        lda8(rd8(ABSX(EFFECT_FRAME)));
        lda8(inc8((u8)A));
        and8(0x03);
        wr8(ABSX(EFFECT_FRAME), (u8)A);
        lda8(0x04); wr8(ABSX(EFFECT_TIMER), 0x04);
    }
    push8(0x81); push16(0x961B); MoveObjectX_m1x1();
    push8((u8)X);
    push8(0x81); push16(0x9620); MoveObjectX8_m1x1();
    X = pull8(); nz8((u8)X);
    lda8((u8)X);
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSX(EFFECT_X))); wr8(ABSY(OAM_BUF + 0x10), (u8)A);
    lda8(rd8(ABSX(EFFECT_Y))); wr8(ABSY(OAM_BUF + 0x11), (u8)A);
    lda8(rd8(ABSX(EFFECT_VY)));
    cmp8((u8)A, 0xF0);
    if (fc) lda8(0x00);
    else lda8(rd8(ABSX(EFFECT_FRAME)));
    push8((u8)X);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(LONGX(0x81966F))); wr8(ABSY(OAM_BUF + 0x12), (u8)A);
    lda8(rd8(LONGX(0x819673))); wr8(ABSY(OAM_BUF + 0x13), (u8)A);
    X = pull8(); nz8((u8)X);
    lda8((u8)Y);
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02); wr8(ABSY(OAM_SIZE_BUF + 0x04), 0x02);
    wr8(ABSX(EFFECT_VY), inc8(rd8(ABSX(EFFECT_VY))));
    lda8(rd8(ABSX(EFFECT_VY)));
    if (fz) {
        lda8(0x03); wr8(ABSX(EFFECT_STEP), 0x03);
        lda8(0x20); wr8(ABSX(EFFECT_TIMER), 0x20);
        wr8(ABSX(EFFECT_FRAME), 0);
    }
    ret();
}
/* $81:9554 (RTS): an empty confetti state. */
void EmptyStep9554_m1x1(void)
{
    ret();
}
/* $80:CA92 (RTL): DrawAllRiders with DB = $80. */
void DrawAllRidersLong_m1x1(void)
{
    push8(DB);
    lda8(0x80); push8(0x80);
    DB = pull8(); nz8(DB);
    push16(0xCA99); DrawAllRiders_m1x1();
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:F128 (RTL, 16-bit A): BuildScoreDigits for other banks. */
void BuildScoreDigitsLong_m0x1(void)
{
    push16(0xF12A); BuildScoreDigits_m0x1();
    retl();
}
/* $80:C394 (RTS): riders 7-1 on screen page 1 ($019E odd) at X $30-$3F
 * flag $092D (cleared elsewhere on that page); Y is left $40 or $80. */
void FlagRidersAtLine_m1x1(void)
{
    ldx8(0x08);
    for (;;) {
        X = dec8((u8)X);
        if (fz) break;
        lda8(rd8(ABSX(RIDER_PAGE)));
        lda8(lsr8((u8)A));
        if (!fc) continue;
        ldy8(0x40);
        lda8(rd8(ABSX(RIDER_X)));
        cmp8((u8)A, 0x30);
        if (fc) {
            ldy8(0x80);
            cmp8((u8)A, 0x40);
            if (!fc) { lda8(0x01); wr8(ABSX(GAUGE_ON), 0x01); continue; }
        }
        lda8(rd8(ABSX(GAUGE_ON)));
        if (fz) continue;
        lda8((u8)Y);
        lda8(0x00);
        wr8(ABSX(GAUGE_ON), 0x00);
    }
    ret();
}
/* $80:AB4F (RTS): on even frames the ball check; on frames 1 mod 4 the
 * line flags and the speed gauge. */
void BallOrGauge_m1x1(void)
{
    lda8(rd8(DP(FRAME_COUNT)));
    lda8(lsr8((u8)A));
    if (!fc) {
        push16(0xAB56); BallHit_m1x1();
        ret();
        return;
    }
    lda8(lsr8((u8)A));
    if (fc) { ret(); return; }
    push16(0xAB5D); FlagRidersAtLine_m1x1();
    SpeedGauge_m1x1();
}
/* $80:D225 (RTS): clear the coin counts $1540-$1542, show the coin rank,
 * then clear the marker objects (ClearWords1530, into which it falls). */
void ResetCoinCounts_m1x1(void)
{
    wr8(ABS(COIN_COMBO), 0); wr8(ABS(MARKER_COUNT), 0); wr8(ABS(COIN_TOTAL), 0);
    push16(0xD230); ShowCoinRank_m1x1();
    ClearWords1530_m1x1();
}
/* $81:C84F (RTL): the jump result's total: the waving player, and when
 * $8C runs out sound $42, the next step ($CD), $8C = $20, and the total
 * (ScaleJumpCount's $0C + the rank points $80:F155) built and queued
 * (QueueResultsTiles, into which it falls). */
void JumpResultTotal_m1x1(void)
{
    ldx8(0x00);
    push8(0x81); push16(0xC854); DrawAllRidersLong_m1x1();
    lda8(0x19); wr8(ABS(SPRITE_FRAME), 0x19);
    u8 t = dec8(rd8(DP(TIMER_B)));
    wr8(DP(TIMER_B), t);
    if (!fz) { retl(); return; }
    lda8(0x42);
    push8(0x81); push16(0xC863); QueueSoundB_m1x1();
    wr8(DP(JUMP_BOARD_STEP), inc8(rd8(DP(JUMP_BOARD_STEP))));
    lda8(0x20); wr8(DP(TIMER_B), 0x20);
    push8(0x81); push16(0xC86D); ScaleJumpCount_m1x1();
    rep(0x20);
    X = rd8(ABS(JUMP_RANK)); nz8((u8)X);
    lda16(rd16(LONGX(0x80F122)));
    and16(0x00FF);
    fc = 0; adc16(rd16(DP(0x0C)));
    wr16(DP(0x0C), A);
    push8(0x81); push16(0xC882); BuildScoreDigitsLong_m0x1();
    QueueResultsTiles_m1x1();
}
/* $81:9538 (RTL): each confetti piece (even slots 6-0) runs its state
 * $0116 through the table $81:954C (Y = slot). X is kept. */
void ConfettiAll_m1x1(void)
{
    push8((u8)X);
    ldx8(0x06);
    do {
        lda8(rd8(ABSX(EFFECT_KIND)));
        lda8(asl8((u8)A));
        push8((u8)X);
        Y = X; nz8((u8)Y);
        X = (u8)A; nz8((u8)X);
        push16(0x9544);
        r816_call_pc(0x810000u | rd16(LONGX(0x81954C)));
        X = pull8(); nz8((u8)X);
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
    X = pull8(); nz8((u8)X);
    retl();
}
void PlayerCrashStep_m1x1(void) { player_crash_step(0); }
void PlayerCrashStepB_m1x1(void) { player_crash_step(1); }
/* $80:BB8B (RTS, X = rider): a rider walking to its start lane steers
 * toward $80:BACF (by rider; lane 4's entry for a CPU rider in the air);
 * on arrival it stops steering, $091C = 0 and steps $01D6; the player
 * also clears $0CDF / $0CE0 ($0990 too when $0CE0 was set), resets the
 * coin counts and plays sound $80. Then $45,X = 8. */
void WalkToStartLane_m1x1(void)
{
    lda8((u8)X);
    Y = (u8)A; nz8((u8)Y);
    if (!fz) {
        lda8(rd8(ABSX(RIDER_HEIGHT)));
        if (!fz) ldy8(0x04);
    }
    lda8(rd8(ABSX(RIDER_LANE)));
    cmp8((u8)A, rd8(ABSY(0xBAD8)));
    if (!fz) {
        ldy8(0x01);
        if (fc) ldy8(0xFF);
        lda8((u8)Y);
        wr8(ABSX(RIDER_STEER), (u8)A);
        ret();
        return;
    }
    lda8(0x00);
    wr8(ABSX(FINISH_POSE), 0);
    wr8(ABSX(RIDER_STEER), 0);
    wr8(ABSX(RIDER_TIMER), inc8(rd8(ABSX(RIDER_TIMER))));
    lda8((u8)X);
    if (fz) {
        Y = rd8(ABS(TURBO_ON)); nz8((u8)Y);
        if (!fz) wr8(ABS(ENGINE_HEAT), (u8)A);
        wr8(ABS(TURBO_ON), (u8)A);
        wr8(ABS(COIN_COUNT), (u8)A);
        push8(0x80); push16(0xBBCA); ResetCoinCounts_m1x1();
        lda8(0x80);
        push8(0x80); push16(0xBBD0); QueueSoundB_m1x1();
        lda8(0x02);
    }
    lda8(0x08);
    wr8(DPX(RIDER_LEAN_TIMER), 0x08);
    ret();
}
/* $80:C587 (RTS): the race clock at VRAM $1897, counting $03B0 up unless
 * the race is over ($84); then TickRaceClock (into which it falls). */
void RaceClockStep_m1x1(void)
{
    lda8(0x97); wr8(DP(0x00), 0x97);
    lda8(0x18); wr8(DP(0x01), 0x18);
    lda8(rd8(DP(RACE_END)));
    if (fz) wr8(ABS(RACE_FRAMES), inc8(rd8(ABS(RACE_FRAMES))));
    TickRaceClock_m1x1();
}
/* $82:82EA (RTL): as QueueTimeDigits, through the font table
 * (ByteTable84BA, lower row +$20, a leading zero minute $0A, the marks
 * $56 / $58), size $28 from $03CF with $0F = $10. X and Y are kept. */
void QueueFontTime_m1x1(void)
{
    push8(DB);
    lda8(0x82); push8(0x82);
    DB = pull8(); nz8(DB);
    push8((u8)X); push8((u8)Y);
    ldx8(0x00); ldy8(0x00);
    lda8(rd8((ILDP(0x18) + Y) & 0xFFFFFF));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        X = inc8((u8)X);
    }
    push8(0x82); push16(0x8303); ByteTable84BA_m1x1();
    wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
    fc = 0; adc8(0x20); wr8(ABS(DIGIT_ROW + 0x09), (u8)A);
    lda8((u8)X);
    cmp8((u8)A, 0x00);
    if (fz) lda8(0x0A);
    push8(0x82); push16(0x8317); ByteTable84BA_m1x1();
    wr8(ABS(DIGIT_ROW), (u8)A);
    fc = 0; adc8(0x20); wr8(ABS(DIGIT_ROW + 0x08), (u8)A);
    static const u16 lo[2] = { 0x03D4, 0x03D7 };
    static const u16 back[2][2] = { { 0x8332, 0x8340 }, { 0x835B, 0x8369 } };
    for (int i = 0; i < 2; i++) {
        Y = inc8((u8)Y);
        ldx8(0x00);
        lda8(rd8((ILDP(0x18) + Y) & 0xFFFFFF));
        for (;;) {
            cmp8((u8)A, 0x0A);
            if (!fc) break;
            sbc8(0x0A);
            X = inc8((u8)X);
        }
        push8(0x82); push16(back[i][0]); ByteTable84BA_m1x1();
        wr8(ABS(lo[i]), (u8)A);
        fc = 0; adc8(0x20); wr8(ABS(lo[i] + 8), (u8)A);
        lda8((u8)X);
        push8(0x82); push16(back[i][1]); ByteTable84BA_m1x1();
        wr8(ABS(lo[i] - 1), (u8)A);
        fc = 0; adc8(0x20); wr8(ABS(lo[i] + 7), (u8)A);
    }
    lda8(0x56); wr8(ABS(DIGIT_ROW + 0x02), 0x56);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABS(DIGIT_ROW + 0x05), (u8)A);
    lda8(0x76); wr8(ABS(DIGIT_ROW + 0x0A), 0x76);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABS(DIGIT_ROW + 0x0D), (u8)A);
    lda8(0x28); wr8(ABS(DIGIT_BUF), 0x28);
    rep(0x20);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x10); wr8(DP(0x0F), 0x10);
    push8(0x82); push16(0x839C); VramQueueTilesLong_m1x1();
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:DDF8 (RTS): the bonus result count-up. A button skips to the final
 * count ($09C6 = $09C5, the nameplate when it is a full score $09C9) and
 * straight on to the result. Otherwise every 4 frames ($88) the shown
 * count $09C6 steps up with sound $44; at the end, next step ($77), a
 * $40 wait and for a full score the banner $80:DE4F at VRAM $18F4. */
void BonusCountUp_m1x1(void)
{
    lda8(rd8(DP(P1_PRESS_HI)));
    ora8(rd8(DP(P1_PRESS_LO)));
    if (!fz) {
        lda8(rd8(ABS(BONUS_RESULT)));
        wr8(ABS(0x09C6), (u8)A);
        wr8(DP(0x10), (u8)A);
        push16(0xDE08); QueueResultsCount_m1x1();
        wr8(DP(MODE_STEP), inc8(rd8(DP(MODE_STEP))));
        lda8(rd8(ABS(BONUS_RESULT)));
        cmp8((u8)A, rd8(ABS(0x09C9)));
        if (fz) { push16(0xDE15); QueueNameplate_m1x1(); }
        bonus_result_body();
        return;
    }
    u8 t = dec8(rd8(DP(MODE_TIMER)));
    wr8(DP(MODE_TIMER), t);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x09C6)));
    cmp8((u8)A, rd8(ABS(BONUS_RESULT)));
    if (!fz) {
        lda8(inc8((u8)A));
        wr8(ABS(0x09C6), (u8)A);
        wr8(DP(0x10), (u8)A);
        lda8(0x44);
        push8(0x80); push16(0xDE30); QueueSoundB_m1x1();
        lda8(0x04); wr8(DP(MODE_TIMER), 0x04);
        QueueResultsCount_m1x1();
        return;
    }
    wr8(DP(MODE_STEP), inc8(rd8(DP(MODE_STEP))));
    lda8(0x40); wr8(DP(MODE_TIMER), 0x40);
    lda8(rd8(ABS(BONUS_RESULT)));
    cmp8((u8)A, rd8(ABS(0x09C9)));
    if (!fz) { ret(); return; }
    rep(0x20);
    lda16(0x18F4); wr16(DP(0x00), 0x18F4);
    lda16(0xDE4F); wr16(DP(0x02), 0xDE4F);
    sep(0x20);
    VramQueueTiles20_m1x1();
}
/* $80:9606 (RTS, 16-bit A on entry): the race results' place line: the
 * place badge at VRAM $1A08 (or the "no time" text $80:9462 at $1A04
 * when $085E is 0), then the place points for place $086E ($80:944A /
 * $80:9452 digits, $BA from $80:945A; a leading zero blank, from 8th
 * place all blank) at $1A17, added into $09AD / $09AE and the score. */
void ResultsPlacePoints_m0x1(void)
{
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    lda8(rd8(ABS(RIDER_DIST)));
    ora8(rd8(ABS(RIDER_DIST + 0x01)));
    if (!fz) {
        rep(0x20);
        lda16(0x1A08); wr16(DP(0x00), 0x1A08);
        sep(0x20);
        push16(0x961F); QueuePlaceBadge_m1x1();
    } else {
        rep(0x20);
        lda16(0x1A04); wr16(DP(0x00), 0x1A04);
        lda16(0x9460); wr16(DP(0x02), 0x9460);
        sep(0x20);
        push16(0x9632); VramQueueTiles_m1x1();
    }
    X = rd8(ABS(PLAYER_PLACE)); nz8((u8)X);
    X = dec8((u8)X);
    lda8(rd8(ABSX(0x9448))); wr8(ABS(DIGIT_ROW), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x05), (u8)A);
    lda8(rd8(ABSX(0x9450))); wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x06), (u8)A);
    lda8(rd8(ABSX(0x9458))); wr8(DP(PLACE_BONUS), (u8)A);
    lda8(0x25); wr8(ABS(DIGIT_BUF), 0x25);
    wr8(ABS(DIGIT_ROW + 0x02), 0); wr8(ABS(DIGIT_ROW + 0x03), 0); wr8(ABS(DIGIT_ROW + 0x04), 0);
    lda8(0x10);
    wr8(ABS(DIGIT_ROW + 0x07), 0x10); wr8(ABS(DIGIT_ROW + 0x08), 0x10); wr8(ABS(DIGIT_ROW + 0x09), 0x10);
    lda8(rd8(ABSX(0x9448)));
    if (fz) {
        lda8(0xFF);
        wr8(ABS(DIGIT_ROW), 0xFF); wr8(ABS(DIGIT_ROW + 0x05), 0xFF);
        cmp8((u8)X, 0x07);
        if (fc) {
            wr8(ABS(DIGIT_ROW + 0x01), 0xFF); wr8(ABS(DIGIT_ROW + 0x06), 0xFF);
            wr8(ABS(DIGIT_ROW + 0x02), 0xFF); wr8(ABS(DIGIT_ROW + 0x07), 0xFF);
            wr8(ABS(DIGIT_ROW + 0x03), 0xFF); wr8(ABS(DIGIT_ROW + 0x08), 0xFF);
        }
    }
    rep(0x20);
    lda16(0x1A17); wr16(DP(0x00), 0x1A17);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    push16(0x969E); VramQueueTiles20_m1x1();
    lda8(rd8(ABS(DIGIT_ROW)));
    if (!fn) wr8(ABS(SCORE_ADD + 0x01), (u8)A);
    lda8(rd8(ABS(DIGIT_ROW + 0x01)));
    if (!fn) wr8(ABS(SCORE_ADD + 0x02), (u8)A);
    results_score_tail();
}
/* $80:96DE (RTS, 16-bit A): the jump results' score line: the score
 * $156D at VRAM $1AB6, its digits 2-5 into $09AD-$09B0 (blanks skipped),
 * then into the score as above. */
void ResultsJumpPoints_m0x1(void)
{
    lda16(rd16(ABS(JUMP_BEST_SCORE)));
    push16(0x96E3); BuildScoreDigits_m0x1();
    rep(0x20);
    lda16(0x1AB6); wr16(DP(0x00), 0x1AB6);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    push16(0x96F4); VramQueueTiles20_m1x1();
    rep(0x20);
    lda16(rd16(ABS(JUMP_BEST_SCORE)));
    push16(0x96FC); BuildScoreDigits_m0x1();
    sep(0x20);
    ldx8(0x02);
    do {
        lda8(rd8(ABSX(DIGIT_BUF)));
        if (!fn) wr8(ABSX(0x09AB), (u8)A);
        X = inc8((u8)X);
        cmp8((u8)X, 0x06);
    } while (!fc);
    results_score_tail();
}
/* $80:C3EA (RTS): while racing ($92) the race time runs: frames $82 (to
 * 60), seconds $81 (to 60) and minutes $80, with the hundredths
 * ($80:C470 -> two digits $80:C4AC at $09B8) and seconds digits ($09B5)
 * and minutes ($09B3) copied to $09BB-$09C1, and the clock shown. At
 * the broadcast's end minute for this race ($80:C468, entry $7C * 2 +
 * $7D; past frame $0708 of it, $CA = 1) or after 4 minutes the race
 * ends: $085E cleared, $84 = 2, sound $2D. */
void RaceTimer_m1x1(void)
{
    lda8(rd8(DP(RACE_STARTED)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(0x82)));
    lda8(inc8((u8)A));
    cmp8((u8)A, 0x3C);
    if (fc) {
        lda8(rd8(DP(0x81)));
        lda8(inc8((u8)A));
        cmp8((u8)A, 0x3C);
        if (fc) { wr8(DP(0x80), inc8(rd8(DP(0x80)))); lda8(0x00); }
        wr8(DP(0x81), (u8)A);
        lda8(0x00);
    }
    wr8(DP(0x82), (u8)A);
    X = (u8)A; nz8((u8)X);
    rep(0x20);
    lda16(rd16(ABSX(0xC479)));
    lda16(asl16(A));
    X = (u8)A; nz8((u8)X);
    lda16(rd16(ABSX(0xC4B5))); wr16(ABS(0x09B8), A);
    lda16(rd16(DP(0x81)));
    lda16(asl16(A));
    X = (u8)A; nz8((u8)X);
    lda16(rd16(ABSX(0xC4B5))); wr16(ABS(0x09B5), A);
    sep(0x20);
    lda8(rd8(DP(0x80))); wr8(ABS(0x09B3), (u8)A);
    ldx8(0x06);
    do {
        lda8(rd8(ABSX(0x09B3)));
        wr8(ABSX(0x09BB), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    push16(0xC433); RaceClockStep_m1x1();
    push8((u8)X);
    lda8(rd8(DP(COURSE_SLOT)));
    lda8(asl8((u8)A));
    fc = 0; adc8(rd8(DP(FINAL_RACE)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0xC471)));
    X = pull8(); nz8((u8)X);
    cmp8((u8)A, rd8(LONG(BCAST_MINUTE)));
    if (fc) {
        if (!fz) goto check_minutes;
        rep(0x20);
        lda16(rd16(LONG(BCAST_FRAME)));
        cmp16(A, 0x0708);
        sep(0x20);
        if (fc) { lda8(0x01); wr8(ABS(0x00CA), 0x01); }
    check_minutes:
        lda8(rd8(DP(0x80)));
        cmp8((u8)A, 0x04);
        if (!fc) { ret(); return; }
    }
    wr8(ABS(RIDER_DIST), 0); wr8(ABS(RIDER_DIST + 0x01), 0);
    lda8(0x02); wr8(DP(RACE_END), 0x02);
    lda8(0x2D);
    push8(0x80); push16(0xC46F); QueueSoundB_m1x1();
    ret();
}
/* $80:AE9C (RTS, X = rider): BrakeRider (into which it falls) on the
 * rider whose turn it is ($7F). */
void BrakeOnTurn_m1x1(void)
{
    cmp8((u8)X, rd8(DP(RIDER_TURN)));
    if (!fz) { ret(); return; }
    BrakeRider_m1x1();
}
/* $80:AC17 (RTS, X = rider): rider X's throttle and lean from its pad
 * $017E. Not crashed, held or changing lanes, on the ground with a button
 * down (A/B: $C0) it accelerates (AccelerateRider) up to the top speed
 * $80:B886 / $80:B883 (by button and, for the player, $84), with sound $3F
 * when the player starts from still; otherwise it brakes (BrakeOnTurn,
 * Y = how hard). Then, not crashed: in the air or fast enough ($A0) the
 * d-pad leans it; on the ground it settles toward the piece angle $08B4. */
void RiderThrottle_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_INPUT)));
    and8(0x03);
    wr8(DP(0x0A), (u8)A);
    ldy8(0x00);
    lda8(rd8(ABSX(RIDER_COUNTDOWN)));
    ora8(rd8(ABSX(RIDER_BUMPED)));
    ora8(rd8(ABSX(0x199E)));
    if (!fz) goto brake;
    lda8(rd8(ABSX(RIDER_TIMER)));
    if (!fz) {
        cmp8((u8)A, 0x05);
        if (!fz) goto brake;
    }
    lda8((u8)X);
    if (fz) {
        ldy8(0x02);
        lda8(rd8(DP(RACE_END)));
        if (!fz) goto top;
        ldy8(0x00);
    }
    lda8(rd8(ABSX(RIDER_AIR)));
    if (!fz) {
        ldy8(0x04);
        lda8(rd8(DP(0x0A)));
        if (fz) goto brake;
        Y = (u8)A; nz8((u8)Y);
        goto harder;
    }
    lda8(rd8(ABSX(RIDER_INPUT)));
    and8(0xC0);
    if (fz) goto brake;
    wr8(DP(0x00), (u8)X);
    cmp8((u8)X, 0x00);
    if (fz) {
        lda8(rd8(ABSX(RIDER_SPEED_LO)));
        ora8(rd8(ABSX(RIDER_SPEED_HI)));
        if (fz) {
            lda8(rd8(DP(P1_PRESS_HI)));
            and8(0xC0);
            if (!fz) { lda8(0x3F); push8(0x80); push16(0xAC64); QueueSoundB_m1x1(); }
        }
    }
    lda8(rd8(ABSX(RIDER_INPUT)));
    lda8(asl8((u8)A));
    if (!fc) wr8(DP(0x00), inc8(rd8(DP(0x00))));
    push16(0xAC6F); RiderOnRamp_m1x1();
    if (!fz) {
        lda8(rd8(ABSX(RIDER_JUMP)));
        if (fz) {
            lda8(rd8(ABSX(RIDER_SPEED_HI)));
            if (!fz) goto brake;
        }
    }
    Y = rd8(DP(0x00)); nz8((u8)Y);
    lda8((u8)X);
    if (!fz) goto accel;
top:
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    cmp8((u8)A, rd8(ABSY(0xB88F)));
    if (!fc) goto accel;
    if (!fz) goto harder;
    lda8(rd8(ABSX(RIDER_SPEED_LO)));
    cmp8((u8)A, rd8(ABSY(0xB88C)));
    if (fz) goto after;
    if (fc) goto harder;
accel:
    push16(0xAC97); AccelerateRider_m1x1();
    goto after;
harder:
    Y = inc8((u8)Y);
brake:
    push16(0xACA5); BrakeOnTurn_m1x1();
after:
    lda8(rd8(ABSX(RIDER_BUMPED)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_AIR)));
    int pad = !fz;
    if (!pad) {
        lda8(rd8(ABSX(RIDER_SCRIPT)));
        ora8(rd8(DP(RACE_END)));
        if (fz) {
            lda8(rd8(ABSX(RIDER_SPEED_HI)));
            pad = !fz;
            if (!pad) {
                lda8(rd8(ABSX(RIDER_SPEED_LO)));
                cmp8((u8)A, 0xA0);
                pad = fc;
            }
        }
    }
    if (pad) {
        lda8(rd8(DP(0x0A)));
        if (!fz) {
            wr8(DP(0x00), (u8)A);
            lean_by_pad();
            return;
        }
    }
    lda8(rd8(ABSX(RIDER_AIR)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_PIECE_LEAN)));
    cmp8((u8)A, rd8(ABSX(RIDER_LEAN)));
    if (fz) { ret(); return; }
    ldy8(0x05);
    lean_toward();
}
/* $80:ABD6 (RTS, X = last rider): the throttle step for riders X-0 on the
 * track ($01DE); crashed, off-track or line-changing riders (not on the
 * $05 start walk) get no pad input. Then, unless $54, the player's
 * button flags $092D (once, not after the race $84). */
void RidersThrottle_m1x1(void)
{
    do {
        lda8(rd8(ABSX(RIDER_ACTIVE)));
        if (!fz) {
            lda8(rd8(ABSX(RIDER_TIMER)));
            cmp8((u8)A, 0x05);
            if (!fz) {
                ora8(rd8(ABSX(RIDER_BUMPED)));
                ora8(rd8(ABSX(OVERHEAT_LOCK)));
                if (!fz) { lda8(0x00); wr8(ABSX(RIDER_INPUT), 0x00); }
            }
            push16(0xABF1); RiderThrottle_m1x1();
        }
        X = dec8((u8)X);
    } while (!fn);
    lda8(rd8(DP(0x54)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(RIDER_INPUT)));
    and8(0xC0);
    if (fz) { ret(); return; }
    lda8(rd8(ABS(GAUGE_ON)));
    ora8(rd8(DP(RACE_END)));
    if (!fz) { ret(); return; }
    lda8(0x01); wr8(ABS(GAUGE_ON), 0x01);
    ret();
}
/* $81:97E8 (RTL): the object list (even slots 14-0, $03A3 = slot): each
 * active object ($02EF) counts its timers $036F / $037F down, runs its
 * type's step ($81:AD8F) and, if still active, its draw ($81:ADA3);
 * $1958 cleared first. */
void UpdateObjects_m1x1(void)
{
    wr8(ABS(SPRITES_FROZEN), 0);
    push8(DB);
    lda8(0x81); push8(0x81);
    DB = pull8(); nz8(DB);
    ldx8(0x0E);
    do {
        wr8(ABS(OBJ_CURRENT), (u8)X);
        lda8(rd8(ABSX(OBJ_ACTIVE)));
        if (!fz) {
            lda8(rd8(ABSX(OBJ_DELAY)));
            if (!fz) wr8(ABSX(OBJ_DELAY), dec8(rd8(ABSX(OBJ_DELAY))));
            lda8(rd8(ABSX(OBJ_COUNTER)));
            if (!fz) wr8(ABSX(OBJ_COUNTER), dec8(rd8(ABSX(OBJ_COUNTER))));
            lda8(rd8(ABSX(OBJ_TYPE)));
            lda8(asl8((u8)A));
            X = (u8)A; nz8((u8)X);
            push8((u8)X);
            push16(0x9812);
            r816_call_pc(0x810000u | rd16(LONGX(0x81AD8F)));
            X = pull8(); nz8((u8)X);
            Y = rd8(ABS(OBJ_CURRENT)); nz8((u8)Y);
            lda8(rd8(ABSY(OBJ_ACTIVE)));
            if (!fz) {
                push16(0x981E);
                r816_call_pc(0x810000u | rd16(LONGX(0x81ADA3)));
            }
        }
        X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:95A0 (RTS): the end-of-broadcast screens. Before the results
 * ($7D = 0) the course records; then the results: objects, the place and
 * jump points lines at timer $04EC / $0474 of $8A, and when $8A runs out
 * (or Start with the debug flag) the next broadcast part: entries 1 and 3
 * go to mode $17 with $1E87 - $0A, the others to the next entry ($7C + 1)
 * in mode $18 with $1905 = $15. */
void BroadcastEndScreens_m1x1(void)
{
    lda8(rd8(DP(FINAL_RACE)));
    if (fz) { course_records_step(); return; }
    push8(0x80); push16(0x95AD); UpdateObjects_m1x1();
    X = rd8(DP(COURSE_SLOT)); nz8((u8)X);
    rep(0x20);
    wr16(DP(TIMER_A), dec16(rd16(DP(TIMER_A))));
    if (!fz) {
        lda16(rd16(DP(TIMER_A)));
        cmp16(A, 0x04EC);
        if (fz) { push16(0x95BF); ResultsPlacePoints_m0x1(); }
        else {
            cmp16(A, 0x0474);
            if (fz) { push16(0x95C9); ResultsJumpPoints_m0x1(); }
        }
        sep(0x20);
        lda8(rd8(LONG(DEBUG_ENABLED)));
        if (fz) { ret(); return; }
        lda8(rd8(DP(P1_PRESS_HI)));
        and8(0x10);
        if (fz) { ret(); return; }
    }
    sep(0x20);
    wr8(DP(MODE_ACTIVE), 0); wr8(DP(MODE_STEP), 0);
    wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
    cmp8((u8)X, 0x03);
    int bonus = fz;
    if (!bonus) { cmp8((u8)X, 0x01); bonus = fz; }
    if (bonus) {
        lda8(0x17); wr8(DP(GAME_MODE), 0x17);
        lda8(rd8(ABS(0x1E87)));
        fc = 1; sbc8(0x0A);
        wr8(ABS(0x1E87), (u8)A);
    } else {
        wr8(DP(FINAL_RACE), 0);
        wr8(DP(COURSE_SLOT), inc8(rd8(DP(COURSE_SLOT))));
        lda8(0x18); wr8(DP(GAME_MODE), 0x18);
        lda8(0x15); wr8(ABS(SH_TM), 0x15);
    }
    ret();
}
/* $82:9028 (RTL): the stadium bonus banner on its timer $D4: at $7E the
 * "bonus" text ($82:9113 at VRAM $1935) and the stadium's bonus points
 * ($82:9131 + 4 * (stadium - 2), leading zeros blank, a trailing 0) at
 * $1977; at 0 both rows are cleared ($82:90F5 / $82:9108). */
void StadiumBonusBanner_m1x1(void)
{
    push8(DB);
    lda8(0x82); push8(0x82);
    DB = pull8(); nz8(DB);
    lda8(rd8(DP(0xD4)));
    if (!fz) {
        wr8(DP(0xD4), dec8(rd8(DP(0xD4))));
        lda8(rd8(DP(0xD4)));
        if (fz) {
            rep(0x20);
            lda16(0x1935); wr16(DP(0x00), 0x1935);
            lda16(0x90F5); wr16(DP(0x02), 0x90F5);
            sep(0x20);
            lda8(0x20); wr8(DP(0x0F), 0x20);
            push8(0x82); push16(0x9056); VramQueueTilesLong_m1x1();
            rep(0x20);
            lda16(0x1977); wr16(DP(0x00), 0x1977);
            lda16(0x9108); wr16(DP(0x02), 0x9108);
            sep(0x20);
            lda8(0x20); wr8(DP(0x0F), 0x20);
            push8(0x82); push16(0x906C); VramQueueTilesLong_m1x1();
        } else {
            cmp8((u8)A, 0x7E);
            if (fz) {
                rep(0x20);
                lda16(0x1935); wr16(DP(0x00), 0x1935);
                lda16(0x9113); wr16(DP(0x02), 0x9113);
                sep(0x20);
                lda8(0x20); wr8(DP(0x0F), 0x20);
                push8(0x82); push16(0x9085); VramQueueTilesLong_m1x1();
                lda8(rd8(ABS(LAP)));
                lda8(dec8((u8)A)); lda8(dec8((u8)A));
                wr8(DP(0x00), (u8)A);
                lda8(asl8((u8)A)); lda8(asl8((u8)A));
                X = (u8)A; nz8((u8)X);
                for (int i = 0; i < 4; i++) {
                    lda8(rd8(ABSX(0x9131 + i)));
                    wr8(ABS(DIGIT_ROW + i), (u8)A);
                    ora8(0x10);
                    wr8(ABS(DIGIT_ROW + 0x05 + i), (u8)A);
                }
                wr8(ABS(DIGIT_ROW + 0x04), 0);
                lda8(0x10); wr8(ABS(DIGIT_ROW + 0x09), 0x10);
                lda8(0x25); wr8(ABS(DIGIT_BUF), 0x25);
                ldx8(0x00);
                for (;;) {
                    lda8(rd8(ABSX(DIGIT_ROW)));
                    if (!fz) break;
                    lda8(0xFF);
                    wr8(ABSX(DIGIT_ROW), 0xFF);
                    wr8(ABSX(DIGIT_ROW + 0x05), 0xFF);
                    X = inc8((u8)X);
                    cmp8((u8)X, 0x02);
                    if (fc) break;
                }
                rep(0x20);
                lda16(0x1977); wr16(DP(0x00), 0x1977);
                lda16(0x03CF); wr16(DP(0x02), 0x03CF);
                sep(0x20);
                lda8(0x20); wr8(DP(0x0F), 0x20);
                push8(0x82); push16(0x90F2); VramQueueTilesLong_m1x1();
            }
        }
    }
    DB = pull8(); nz8(DB);
    retl();
}
/* $82:8CF0 SpeedReadout (RTL): on odd frames the player's speed
 * ($01C6:$01BE; while it holds steady and nonzero it jitters by a random
 * 0 / 1, $D0 = last) as "NNN.N" at VRAM $1B22 (leading zeros blank,
 * the point $54); 0 before the jump contest's first run ($C7 = 2 with
 * $1569 = 0). */
void SpeedReadout_m1x1(void)
{
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x01);
    if (fz) { retl(); return; }
    lda8(rd8(ABS(RIDER_SPEED_HI)));
    A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
    lda8(rd8(ABS(RIDER_SPEED_LO)));
    rep(0x20);
    cmp16(A, rd16(DP(0xD0)));
    int jitter = fz;
    if (jitter) { cmp16(A, 0x0000); jitter = !fz; }
    if (jitter) {
        wr16(DP(0xD0), A);
        sep(0x20);
        push8(0x82); push16(0x8D10); RandomB_m1x1();
        rep(0x20);
        and16(0x0001);
        fc = 0; adc16(rd16(DP(0xD0)));
    } else {
        wr16(DP(0xD0), A);
    }
    wr16(DP(0x00), 0); wr16(DP(0x02), 0); wr16(DP(0x04), 0);
    wr16(DP(0x0E), A);
    lda16(rd16(ABS(JUMP_STATE)));
    and16(0x00FF);
    cmp16(A, 0x0002);
    if (fz) {
        lda16(rd16(ABS(JUMP_COUNT)));
        if (fz) wr16(DP(0x0E), 0);
    }
    lda16(rd16(DP(0x0E)));
    static const u16 pow10[3] = { 1000, 100, 10 };
    static const u16 cnt[3] = { 0x00, 0x02, 0x04 };
    for (int i = 0; i < 3; i++) {
        for (;;) {
            cmp16(A, pow10[i]);
            if (!fc) break;
            sbc16(pow10[i]);
            wr16(DP(cnt[i]), inc16(rd16(DP(cnt[i]))));
            WAIT_TICK();
        }
    }
    sep(0x20);
    wr8(ABS(DIGIT_ROW + 0x04), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x09), (u8)A);
    lda8(0xFF); wr8(ABS(DIGIT_ROW + 0x03), 0xFF);
    lda8(0x54); wr8(ABS(DIGIT_ROW + 0x08), 0x54);
    lda8(rd8(DP(0x04)));
    wr8(ABS(DIGIT_ROW + 0x02), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x07), (u8)A);
    lda8(rd8(DP(0x00)));
    if (fz) {
        lda8(0xFF); wr8(ABS(DIGIT_ROW), 0xFF); wr8(ABS(DIGIT_ROW + 0x05), 0xFF);
    } else {
        wr8(ABS(DIGIT_ROW), (u8)A);
        ora8(0x10); wr8(ABS(DIGIT_ROW + 0x05), (u8)A);
    }
    lda8(rd8(DP(0x02)));
    int blank = 0;
    if (fz) { Y = rd8(DP(0x00)); nz8((u8)Y); blank = fz; }
    if (blank) {
        lda8(0xFF); wr8(ABS(DIGIT_ROW + 0x01), 0xFF); wr8(ABS(DIGIT_ROW + 0x06), 0xFF);
    } else {
        wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
        ora8(0x10); wr8(ABS(DIGIT_ROW + 0x06), (u8)A);
    }
    lda8(0x25); wr8(ABS(DIGIT_BUF), 0x25);
    rep(0x20);
    lda16(0x1B22); wr16(DP(0x00), 0x1B22);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push8(0x82); push16(0x8DC5); VramQueueTilesLong_m1x1();
    retl();
}
/* $80:E864 (RTS): the jump contest's frame. Waiting for the next run
 * ($C7 = 2) the rider brakes and is drawn standing ($0216 = $35). Then
 * the engine sound, the countdown lights, $A0 = distance / 2 and the
 * start timer $88 (sound $26 at $40; at 0, $8E = $80, sound $27, the
 * run count $C3 + 1). Unless paused ($54) or past the end ($84 even
 * and nonzero: to mode $0A with $50 = 2), the player's pad drives the
 * rider, the twenty jump steps run ($85 = 0-$13), and the speed readout
 * and scroll update. */
void JumpContestFrame_m1x1(void)
{
    lda8(rd8(ABS(JUMP_STATE)));
    cmp8((u8)A, 0x02);
    if (fz) {
        ldy8(0x05);
        push16(0xE86F); BrakeOnTurn_m1x1();
        lda8(0x01); wr8(ABS(RIDER_ON_RAMP), 0x01);
        ldx8(0x00);
        Y = X; nz8((u8)Y);
        lda8(0x35); wr8(ABS(RIDER_TILE_Y), 0x35);
        push16(0xE87F); DrawJumpRiderSprites_m1x1();
    }
    push16(0xE882); SendSpeedToApu_m1x1();
    push16(0xE885); DrawCountdownLights_m1x1();
    rep(0x20);
    lda16(rd16(DP(SCROLL_X)));
    lda16(lsr16(A));
    wr16(DP(0xA0), A);
    lda16(rd16(DP(MODE_TIMER)));
    if (!fz) {
        cmp16(A, 0x0040);
        if (fz) {
            lda16(0x0026);
            push8(0x80); push16(0xE89C); QueueSoundB_m0x1();
        }
        wr16(DP(MODE_TIMER), dec16(rd16(DP(MODE_TIMER))));
        lda16(rd16(DP(MODE_TIMER)));
        if (!fz) { sep(0x20); ret(); return; }
        lda16(0x0080); wr16(DP(LIGHTS_TIMER), 0x0080);
        sep(0x20);
        lda8(0x27);
        push8(0x80); push16(0xE8AF); QueueSoundB_m1x1();
        wr8(DP(COUNTDOWN), inc8(rd8(DP(COUNTDOWN))));
        wr8(ABS(SPRITES_FROZEN), 0);
    }
    sep(0x20);
    lda8(rd8(DP(0x54)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(RACE_END)));
    if (!fz) {
        lda8(lsr8((u8)A));
        if (!fc) {
            lda8(0x02); wr8(DP(MODE_HOLD), 0x02);
            lda8(0x00); wr8(DP(SUB_STEP), 0x00);
            lda8(0x0A);
            ChangeMode_m1x1();
            return;
        }
    }
    lda8(rd8(DP(P1_HELD_HI)));
    wr8(ABS(RIDER_INPUT), (u8)A);
    lda8(0x00); wr8(DP(SUB_STEP), 0x00);
    do {
        push16(0xE8CD); DispatchBy85B_m1x1();
        wr8(DP(SUB_STEP), inc8(rd8(DP(SUB_STEP))));
        lda8(rd8(DP(SUB_STEP)));
        cmp8((u8)A, 0x14);
    } while (!fc);
    push8(0x80); push16(0xE8D9); SpeedReadout_m1x1();
    push8(0x80); push16(0xE8DD); SetJumpScroll_m1x1();
    sep(0x20);
    ret();
}
/* $80:FEC9 (RTL): QueueBonusDigits for other banks. */
void QueueBonusDigitsLong_m1x1(void)
{
    push16(0xFECB); QueueBonusDigits_m1x1();
    retl();
}
/* $80:C622 (RTL): QueueBestTime with DB = $80. */
void QueueBestTimeLong_m1x1(void)
{
    push8(DB);
    lda8(0x80); push8(0x80);
    DB = pull8(); nz8(DB);
    push16(0xC629); QueueBestTime_m1x1();
    DB = pull8(); nz8(DB);
    retl();
}
/* $82:8DC7 (RTL): the new-record blink on its timer $CB: at 1 the best
 * time back at VRAM $18D7; every 16 frames the clock at $1897 alternates
 * between blank and the stadium's record ($0819 + 3 * (stadium - 2):
 * minute, seconds, hundredths via $80:C470; the marks $47 / $48). */
void BestTimeBlink_m1x1(void)
{
    lda8(rd8(DP(CLOCK_HIDE)));
    if (fz) { retl(); return; }
    wr8(DP(CLOCK_HIDE), dec8(rd8(DP(CLOCK_HIDE))));
    if (fz) {
        lda8(0xD7); wr8(DP(0x00), 0xD7);
        lda8(0x18); wr8(DP(0x01), 0x18);
        lda8(0x24); wr8(DP(0x0F), 0x24);
        push8(0x82); push16(0x8DDF); QueueBestTimeLong_m1x1();
    }
    lda8(rd8(DP(CLOCK_HIDE)));
    and8(0x0F);
    if (!fz) { retl(); return; }
    lda8(rd8(DP(CLOCK_HIDE)));
    and8(0x10);
    if (fz) {
        ldx8(0x0F);
        lda8(0xFF);
        do {
            wr8(ABSX(DIGIT_BUF), 0xFF);
            X = dec8((u8)X);
        } while (!fn);
    } else {
        lda8(rd8(ABS(LAP)));
        lda8(dec8((u8)A)); lda8(dec8((u8)A));
        wr8(DP(0x00), (u8)A);
        lda8(asl8((u8)A));
        fc = 0; adc8(rd8(DP(0x00)));
        X = (u8)A; nz8((u8)X);
        Y = rd8(ABSX(0x081B)); nz8((u8)Y);
        lda8(rd8(ABSY(0xC479)));
        static const u16 lo[2] = { 0x03D6, 0x03D3 };
        for (int i = 0; i < 2; i++) {
            if (i) lda8(rd8(ABSX(0x081A)));
            ldy8(0x00);
            for (;;) {
                cmp8((u8)A, 0x0A);
                if (!fc) break;
                sbc8(0x0A);
                Y = inc8((u8)Y);
            }
            wr8(ABS(lo[i]), (u8)A);
            ora8(0x10); wr8(ABS(lo[i] + 7), (u8)A);
            lda8((u8)Y);
            wr8(ABS(lo[i] - 1), (u8)A);
            ora8(0x10); wr8(ABS(lo[i] + 6), (u8)A);
        }
        lda8(rd8(ABSX(0x0819)));
        wr8(ABS(DIGIT_ROW), (u8)A);
        ora8(0x10); wr8(ABS(DIGIT_ROW + 0x07), (u8)A);
        lda8(0xFF); wr8(ABS(DIGIT_ROW + 0x08), 0xFF); wr8(ABS(DIGIT_ROW + 0x0B), 0xFF);
        lda8(0x47); wr8(ABS(DIGIT_ROW + 0x01), 0x47);
        lda8(0x48); wr8(ABS(DIGIT_ROW + 0x04), 0x48);
    }
    rep(0x20);
    lda16(0x1897); wr16(DP(0x00), 0x1897);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x27); wr8(ABS(DIGIT_BUF), 0x27);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push8(0x82); push16(0x8E7D); VramQueueTilesLong_m1x1();
    retl();
}
/* $81:9926 (RTS): the results' bonus count-down object. Every 4 frames
 * ($88) the bonus $BA counts down by one into the score (sound $44, the
 * count at VRAM $1CB6, $09AF = 1, the bonus digits). At 0 it hands the
 * podium on: the player's place (the slot where $086E is 1) plus 8 *
 * $154F picks the type-7 object's state 3 (timer $10) and the type from
 * $81:9A06 goes to state 4 with long timers; its own state steps. */
void ResultsBonusCount_m1x1(void)
{
    lda8(rd8(DP(MODE_TIMER)));
    if (!fz) goto tick;
    lda8(rd8(DP(PLACE_BONUS)));
    if (fz) {
        ldy8(0x0E);
        do {
            lda8(rd8(ABSY(PLAYER_PLACE)));
            lda8(dec8((u8)A));
            if (fz) break;
            Y = dec8((u8)Y);
            Y = dec8((u8)Y);
        } while (!fn);
        lda8((u8)Y);
        lda8(lsr8((u8)A));
        wr8(DP(0x00), (u8)A);
        lda8(rd8(ABS(PLAYER_CHAR)));
        lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
        fc = 0; adc8(rd8(DP(0x00)));
        wr8(DP(0x00), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        lda8(0x03); wr8(DP(0x01), 0x03);
        lda8(0x07);
        push8(0x81); push16(0x9956); FindObjectSlot_m1x1();
        lda8(rd8(DP(0x01))); wr8(ABSY(OBJ_STATE), (u8)A);
        lda8(0x10); wr8(ABSY(OBJ_DELAY), 0x10);
        X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
        wr8(ABSX(OBJ_STATE), inc8(rd8(ABSX(OBJ_STATE))));
        Y = rd8(DP(0x00)); nz8((u8)Y);
        lda8(rd8(ABSY(0x9A06)));
        push8(0x81); push16(0x996F); FindObjectSlot_m1x1();
        lda8(0x04); wr8(ABSY(OBJ_STATE), 0x04);
        lda8(0xFF); wr8(ABSY(OBJ_DELAY), 0xFF);
        lda8(0xFF); wr8(ABSY(OBJ_COUNTER), 0xFF);
        goto tick;
    }
    wr8(DP(PLACE_BONUS), dec8(rd8(DP(PLACE_BONUS))));
    lda8(0x44);
    push8(0x81); push16(0x9989); QueueSoundB_m1x1();
    ldy8(0x00);
    lda8(rd8(DP(PLACE_BONUS)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
    }
    wr8(DP(0x00), (u8)A);
    wr8(ABS(DIGIT_ROW + 0x02), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x07), (u8)A);
    cmp8((u8)Y, 0x00);
    if (!fz) {
        lda8((u8)Y);
        wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
        ora8(0x10); wr8(ABS(DIGIT_ROW + 0x06), (u8)A);
    } else {
        lda8(0xFF); wr8(ABS(DIGIT_ROW + 0x01), 0xFF); wr8(ABS(DIGIT_ROW + 0x06), 0xFF);
    }
    lda8((u8)Y);
    ora8(rd8(DP(0x00)));
    if (fz) {
        lda8(0xFF);
        wr8(ABS(DIGIT_ROW + 0x02), 0xFF); wr8(ABS(DIGIT_ROW + 0x03), 0xFF);
        wr8(ABS(DIGIT_ROW + 0x07), 0xFF); wr8(ABS(DIGIT_ROW + 0x08), 0xFF);
    }
    lda8(0xFF); wr8(ABS(DIGIT_ROW), 0xFF); wr8(ABS(DIGIT_ROW + 0x05), 0xFF);
    lda8(0x04); wr8(DP(MODE_TIMER), 0x04);
    rep(0x20);
    lda16(0x1CB6); wr16(DP(0x00), 0x1CB6);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x28); wr8(DP(0x0F), 0x28);
    push8(0x81); push16(0x99EC); VramQueueTilesLong_m1x1();
    lda8(0x01); wr8(ABS(SCORE_ADD + 0x03), 0x01);
    push8(0x81); push16(0x99F5); AddScoreLong_m1x1();
    wr8(DP(0x00), dec8(rd8(DP(0x00))));
    lda8(0x28); wr8(DP(0x0F), 0x28);
    push8(0x81); push16(0x99FF); QueueBonusDigitsLong_m1x1();
tick:
    wr8(DP(MODE_TIMER), dec8(rd8(DP(MODE_TIMER))));
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    ret();
}
/* $80:F3AA (RTS): the attract demo's pad display: a 2x2 d-pad sprite
 * (OAM $1680, tiles from $80:F4BF by the direction table $80:F4AF on
 * $098E's low nibble, attribute $26) and a 4-tile button sprite (OAM
 * $1670, tiles from $80:F4C4 by $098E bits 7-6, attribute $28), both
 * large; then PUSH START. */
void DemoPadDisplay_m1x1(void)
{
    lda8(rd8(ABS(DEMO_PAD)));
    and8(0x0F);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xF47C)));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xF48C)));
    wr8(DP(0x00), (u8)A);
    ldy8(0x80);
    lda8(0x80);
    wr8(ABSY(OAM_BUF), (u8)A); wr8(ABSY(OAM_BUF + 0x08), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(OAM_BUF + 0x04), (u8)A); wr8(ABSY(OAM_BUF + 0x0C), (u8)A);
    lda8(0x18);
    wr8(ABSY(OAM_BUF + 0x01), (u8)A); wr8(ABSY(OAM_BUF + 0x05), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(OAM_BUF + 0x09), (u8)A); wr8(ABSY(OAM_BUF + 0x0D), (u8)A);
    lda8(rd8(DP(0x00)));
    wr8(ABSY(OAM_BUF + 0x02), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A));
    wr8(ABSY(OAM_BUF + 0x06), (u8)A);
    fc = 0; adc8(0x1E);
    wr8(ABSY(OAM_BUF + 0x0A), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A));
    wr8(ABSY(OAM_BUF + 0x0E), (u8)A);
    lda8(0x26);
    wr8(ABSY(OAM_BUF + 0x03), 0x26); wr8(ABSY(OAM_BUF + 0x07), 0x26);
    wr8(ABSY(OAM_BUF + 0x0B), 0x26); wr8(ABSY(OAM_BUF + 0x0F), 0x26);
    lda8((u8)Y);
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02);
    wr8(ABSY(OAM_SIZE_BUF), 0x02); wr8(ABSY(OAM_SIZE_BUF + 0x01), 0x02);
    wr8(ABSY(OAM_SIZE_BUF + 0x02), 0x02); wr8(ABSY(OAM_SIZE_BUF + 0x03), 0x02);
    lda8(rd8(ABS(DEMO_PAD)));
    lda8(rol8((u8)A)); lda8(rol8((u8)A)); lda8(rol8((u8)A));
    and8(0x03);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xF491)));
    wr8(DP(0x00), (u8)A);
    ldy8(0x70);
    lda8(0xB0);
    wr8(ABSY(OAM_BUF), (u8)A); wr8(ABSY(OAM_BUF + 0x08), (u8)A);
    fc = 0; adc8(0x18);
    wr8(ABSY(OAM_BUF + 0x04), (u8)A); wr8(ABSY(OAM_BUF + 0x0C), (u8)A);
    lda8(0x18);
    wr8(ABSY(OAM_BUF + 0x01), (u8)A); wr8(ABSY(OAM_BUF + 0x05), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(OAM_BUF + 0x09), (u8)A); wr8(ABSY(OAM_BUF + 0x0D), (u8)A);
    lda8(rd8(DP(0x00)));
    wr8(ABSY(OAM_BUF + 0x02), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(OAM_BUF + 0x06), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(OAM_BUF + 0x0A), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(OAM_BUF + 0x0E), (u8)A);
    lda8(0x28);
    wr8(ABSY(OAM_BUF + 0x03), 0x28); wr8(ABSY(OAM_BUF + 0x07), 0x28);
    wr8(ABSY(OAM_BUF + 0x0B), 0x28); wr8(ABSY(OAM_BUF + 0x0F), 0x28);
    lda8((u8)Y);
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02);
    wr8(ABSY(OAM_SIZE_BUF), 0x02); wr8(ABSY(OAM_SIZE_BUF + 0x01), 0x02);
    wr8(ABSY(OAM_SIZE_BUF + 0x02), 0x02); wr8(ABSY(OAM_SIZE_BUF + 0x03), 0x02);
    push16(0xF47A); TitlePushStart_m1x1();
    ret();
}
/* $80:F0D4 (RTS): the jump contest's HUD numbers: the last distance $1567
 * (half-units, $81:C8E1) at VRAM $1856, and the best score $156D (raised
 * to ScaleJumpCount's $0C + the rank points when higher) at $1897. */
void JumpHudNumbers_m1x1(void)
{
    rep(0x20);
    lda16(0x1856); wr16(DP(0x00), 0x1856);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    lda16(0x0020); wr16(DP(0x0F), 0x0020);
    lda16(rd16(ABS(JUMP_LAST_DIST)));
    push8(0x80); push16(0xF0EB); QueueHalfUnits_m0x1();
    push8(0x80); push16(0xF0EF); ScaleJumpCount_m1x1();
    rep(0x20);
    Y = rd8(ABS(JUMP_RANK)); nz8((u8)Y);
    lda16(rd16(ABSY(0xF122)));
    and16(0x00FF);
    fc = 0; adc16(rd16(DP(0x0C)));
    cmp16(A, rd16(ABS(JUMP_BEST_SCORE)));
    if (fc) wr16(ABS(JUMP_BEST_SCORE), A);
    lda16(rd16(ABS(JUMP_BEST_SCORE)));
    push16(0xF10B); BuildScoreDigits_m0x1();
    rep(0x20);
    lda16(0x1897); wr16(DP(0x00), 0x1897);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xF120); VramQueueTiles_m1x1();
    ret();
}
/* $80:E5B9 (RTS): set up the jump contest: rider palettes; the lane rows
 * ($09D3 ramps $FC, three empty rows, six scenery rows $3B); the streamer
 * start; the player at column 9, X $30, angle 6 on page 1 ($94 = 2,
 * $1905 = $17, HDMA scene 3), drawn once at ground $80 then $B8; HDMA
 * channels 6/7 ($82:97B1 -> $210D, $82:9AB2 -> $210F); BG offsets $0100,
 * $88 = $78, the line buffer $1000-$13FF cleared, scores cleared,
 * $190F = $DC; the bonus columns and the HUD numbers; $156B = $FF and
 * BG34NBA = 1. */
void SetupJumpContest_m1x1(void)
{
    push8(0x80); push16(0xE5BC); LoadRiderPalettes_m1x1();
    ldx8(0x3F);
    do {
        lda8(0xFC); wr8(ABSX(TRACK_ROWS), 0xFC);
        lda8(0x00);
        wr8(ABSX(TRACK_ROWS + 0x40), 0); wr8(ABSX(TRACK_ROWS + 0x80), 0); wr8(ABSX(TRACK_ROWS + 0xC0), 0);
        lda8(0x3B);
        wr8(ABSX(TRACK_ROWS + 0x180), 0x3B); wr8(ABSX(TRACK_ROWS + 0x1C0), 0x3B); wr8(ABSX(TRACK_ROWS + 0x200), 0x3B);
        wr8(ABSX(TRACK_ROWS + 0x240), 0x3B); wr8(ABSX(TRACK_ROWS + 0x280), 0x3B); wr8(ABSX(TRACK_ROWS + 0x2C0), 0x3B);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x34); wr8(ABS(TRACK_ROW_PTR), 0x34);
    lda8(0x0A); wr8(ABS(0x0279), 0x0A);
    lda8(0x05); wr8(ABS(TRACK_VRAM_HALF), 0x05);
    lda8(0x74); wr8(ABS(TRACK_VRAM_COL), 0x74);
    lda8(0x08); wr8(ABS(RIDER_WHEEL), 0x08);
    lda8(0x09); wr8(ABS(RIDER_COLUMN), 0x09);
    lda8(0x30); wr8(ABS(RIDER_X), 0x30);
    lda8(0x06); wr8(ABS(RIDER_LEAN), 0x06);
    lda8(0x01);
    wr8(ABS(RIDER_PAGE), 0x01); wr8(ABS(RIDER_ACTIVE), 0x01); wr8(ABS(RIDER_BAND), 0x01);
    lda8(asl8((u8)A));
    wr8(DP(0x94), (u8)A);
    lda8(0x17); wr8(ABS(SH_TM), 0x17);
    lda8(0x03);
    push8(0x80); push16(0xE626); LoadHdmaScene_m1x1();
    ldx8(0x00);
    lda8(0x80); wr8(ABS(GROUND_LINE), 0x80); wr8(ABS(RIDER_Y), 0x80);
    push16(0xE633); DrawAllRiders_m1x1();
    lda8(0xB8); wr8(ABS(GROUND_LINE), 0xB8); wr8(ABS(RIDER_Y), 0xB8);
    static const u8 ch7[6][2] = { {0x70,0x42},{0x71,0x0F},{0x72,0xB2},{0x73,0x9A},{0x74,0x82},{0x77,0x00} };
    static const u8 ch6[6][2] = { {0x60,0x42},{0x61,0x0D},{0x62,0xB1},{0x63,0x97},{0x64,0x82},{0x67,0x00} };
    for (int i = 0; i < 6; i++) { lda8(ch7[i][1]); wr8(ABS(0x4300 | ch7[i][0]), ch7[i][1]); }
    for (int i = 0; i < 6; i++) { lda8(ch6[i][1]); wr8(ABS(0x4300 | ch6[i][0]), ch6[i][1]); }
    wr8(ABS(PARALLAX_ON), inc8(rd8(ABS(PARALLAX_ON))));
    rep(0x30);
    lda16(0x0100); wr16(DP(BG1_VSCROLL), 0x0100); wr16(DP(ZOOM), 0x0100);
    lda16(0x0078); wr16(DP(MODE_TIMER), 0x0078);
    X = 0x03FE; nz16(X);
    do {
        wr16(ABSX(0x1000), 0);
        X = dec16(X);
        X = dec16(X);
    } while (!fn);
    wr16(ABS(0x1993), 0); wr16(ABS(0x1997), 0);
    sep(0x30);
    wr8(ABS(0x1999), 0);
    lda8(0xDC); wr8(ABS(SH_HDMAEN), 0xDC);
    push16(0xE6A5); FillBonusColumns_m1x1();
    push16(0xE6A8); JumpHudNumbers_m1x1();
    lda8(0xFF); wr8(ABS(JUMP_RANK), 0xFF);
    lda8(0x01); wr8(ABS(BG34NBA), 0x01);
    ret();
}
/* $80:F04B (RTS): the last jump distance $1567 (half-units) at VRAM
 * $1A75. */
void ShowLastDistance_m1x1(void)
{
    rep(0x20);
    lda16(0x1A75); wr16(DP(0x00), 0x1A75);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    lda16(0x0020); wr16(DP(0x0F), 0x0020);
    lda16(rd16(ABS(JUMP_LAST_DIST)));
    push8(0x80); push16(0xF062); QueueHalfUnits_m0x1();
    ret();
}
/* $81:C7EF (RTL): the jump result's distance step: sound $42, next step
 * ($CD), $8C = $20, this run's distance $1569 at VRAM $1932 and its
 * label rows ($81:C82F, $0208) at $1927. */
void JumpResultDistance_m1x1(void)
{
    lda8(0x42);
    push8(0x81); push16(0xC7F4); QueueSoundB_m1x1();
    wr8(DP(JUMP_BOARD_STEP), inc8(rd8(DP(JUMP_BOARD_STEP))));
    lda8(0x20); wr8(DP(TIMER_B), 0x20);
    rep(0x20);
    lda16(0x1932); wr16(DP(0x00), 0x1932);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    lda16(0x0020); wr16(DP(0x0F), 0x0020);
    lda16(rd16(ABS(JUMP_COUNT)));
    push8(0x81); push16(0xC812); QueueHalfUnits_m0x1();
    rep(0x20);
    lda16(0x1927); wr16(DP(0x00), 0x1927);
    lda16(0xC82F); wr16(DP(0x02), 0xC82F);
    ldx8(0x81); wr8(DP(0x04), 0x81);
    lda16(0x0208); wr16(DP(0x10), 0x0208);
    sep(0x20);
    push8(0x81); push16(0xC82D); QueueTileRowsLong_m1x1();
    retl();
}
/* $80:EA71 (RTS): the jump result board: best distance ($1567) at $1856,
 * the best score at $1897 (this run's when the board is still counting,
 * $88 = 0), this run's distance at $1932 and score parts at $1970 /
 * $19B3 / $19F3; a new best ($1569 > $1567 or score > $156D) blinks with
 * $8A and, at $88 = $3B ($7F without $1999), plays sound $43. */
void JumpResultBoard_m1x1(void)
{
    wr8(DP(0xD6), 0);
    rep(0x20);
    lda16(0x1856); wr16(DP(0x00), 0x1856);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    lda16(0x0020); wr16(DP(0x0F), 0x0020);
    lda16(rd16(ABS(JUMP_LAST_DIST)));
    push8(0x80); push16(0xEA8A); QueueHalfUnits_m0x1();
    rep(0x20);
    lda16(rd16(DP(MODE_TIMER)));
    int best = !fz;
    if (!best) {
        push8(0x80); push16(0xEA94); ScaleJumpCount_m0x1();
        Y = rd8(ABS(JUMP_RANK)); nz8((u8)Y);
        lda16(rd16(ABSY(0xF122)));
        and16(0x00FF);
        fc = 0; adc16(rd16(DP(0x0C)));
        cmp16(A, rd16(ABS(JUMP_BEST_SCORE)));
        best = !fc;
    }
    if (best) lda16(rd16(ABS(JUMP_BEST_SCORE)));
    push16(0xEAAB); BuildScoreDigits_m0x1();
    rep(0x20);
    lda16(0x1897); wr16(DP(0x00), 0x1897);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xEAC0); VramQueueTiles_m1x1();
    rep(0x20);
    lda16(0x1932); wr16(DP(0x00), 0x1932);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    lda16(rd16(ABS(JUMP_LAST_DIST)));
    cmp16(A, rd16(ABS(JUMP_COUNT)));
    if (fc) lda16(0x0020);
    else {
        wr16(DP(0xD6), inc16(rd16(DP(0xD6))));
        lda16(rd16(DP(TIMER_A)));
    }
    wr16(DP(0x0F), A);
    lda16(rd16(ABS(JUMP_COUNT)));
    push8(0x80); push16(0xEAE6); QueueHalfUnits_m0x1();
    rep(0x20);
    lda16(rd16(ABS(JUMP_COUNT)));
    push16(0xEAEE); BuildScoreDigits_m0x1();
    push8(0x80); push16(0xEAF2); BuildJumpScore_m1x1();
    rep(0x20);
    lda16(0x1970); wr16(DP(0x00), 0x1970);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xEB07); VramQueueTiles_m1x1();
    rep(0x20);
    Y = rd8(ABS(JUMP_RANK)); nz8((u8)Y);
    lda16(rd16(ABSY(0xF122)));
    and16(0x00FF);
    push16(0xEB15); BuildScoreDigits_m0x1();
    rep(0x20);
    lda16(0x19B3); wr16(DP(0x00), 0x19B3);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xEB2A); VramQueueTiles_m1x1();
    push8(0x80); push16(0xEB2E); ScaleJumpCount_m1x1();
    rep(0x20);
    Y = rd8(ABS(JUMP_RANK)); nz8((u8)Y);
    lda16(rd16(ABSY(0xF122)));
    and16(0x00FF);
    fc = 0; adc16(rd16(DP(0x0C)));
    wr16(DP(0x0C), A);
    push16(0xEB41); BuildScoreDigits_m0x1();
    rep(0x20);
    lda16(0x19F3); wr16(DP(0x00), 0x19F3);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    lda16(rd16(ABS(JUMP_BEST_SCORE)));
    cmp16(A, rd16(DP(0x0C)));
    sep(0x20);
    if (fc) lda8(0x20);
    else {
        wr8(DP(0xD6), inc8(rd8(DP(0xD6))));
        lda8(rd8(DP(TIMER_A)));
    }
    wr8(DP(0x0F), (u8)A);
    push16(0xEB63); VramQueueTiles_m1x1();
    lda8(rd8(ABS(0x1999)));
    u16 mark = fz ? 0x007F : 0x003B;
    rep(0x20);
    lda16(rd16(DP(MODE_TIMER)));
    cmp16(A, mark);
    sep(0x20);
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0xD6)));
    if (fz) { ret(); return; }
    lda8(0x43);
    push8(0x80); push16(0xEB8A); QueueSoundB_m1x1();
    ret();
}
/* $80:EA0A (RTS): the jump result board step: while $88 counts down, the
 * new-best blink $8A ($24 steady with $1999, else $20 / $24 every 16
 * frames); at 0 the best distance is kept ($1567 = max with $1569),
 * $8A = $20 and the step $CD advances. Then the waving player and the
 * board. */
void JumpResultBoardStep_m1x1(void)
{
    lda8(rd8(DP(MODE_TIMER)));
    if (!fz) {
        lda8(dec8((u8)A));
        wr8(DP(MODE_TIMER), (u8)A);
        X = rd8(ABS(0x1999)); nz8((u8)X);
        if (!fz) lda8(0x24);
        else {
            and8(0x10);
            lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
            ora8(0x20);
        }
        wr8(DP(TIMER_A), (u8)A);
    } else {
        rep(0x20);
        lda16(rd16(ABS(JUMP_COUNT)));
        cmp16(A, rd16(ABS(JUMP_LAST_DIST)));
        if (fc) wr16(ABS(JUMP_LAST_DIST), A);
        sep(0x20);
        lda8(0x20); wr8(DP(TIMER_A), 0x20);
        wr8(DP(JUMP_BOARD_STEP), inc8(rd8(DP(JUMP_BOARD_STEP))));
    }
    ldx8(0x00);
    push16(0xEA3D); DrawAllRiders_m1x1();
    lda8(0x19); wr8(ABS(SPRITE_FRAME), 0x19);
    push16(0xEA45); JumpResultBoard_m1x1();
    ret();
}
/* $80:E941 (RTL): ShowJumpCount for other banks. */
void ShowJumpCountLong_m1x1(void)
{
    push16(0xE943); ShowJumpCount_m1x1();
    retl();
}
/* $80:E99D (RTL): ShowBonusPoints for other banks. */
void ShowBonusPointsLong_m1x1(void)
{
    push16(0xE99F); ShowBonusPoints_m1x1();
    retl();
}
/* $81:C956 (RTL): a button on the jump result screen skips ahead: from
 * step $CD 2-5 the remaining parts (distance, count, bonus points, the
 * total) are shown at once; then step 5, $8C = 2, $88 = $3C and
 * $1999 = 1. */
void JumpResultSkip_m1x1(void)
{
    lda8(rd8(DP(P1_PRESS_HI)));
    ora8(rd8(DP(P1_PRESS_LO)));
    if (fz) { retl(); return; }
    lda8(rd8(DP(JUMP_BOARD_STEP)));
    cmp8((u8)A, 0x02);
    if (fn) { retl(); return; }
    int from = 2;
    if (!fz) {
        cmp8((u8)A, 0x03);
        if (fz) from = 3;
        else {
            cmp8((u8)A, 0x04);
            if (fz) from = 4;
            else {
                cmp8((u8)A, 0x05);
                if (fz) from = 5;
                else {
                    cmp8((u8)A, 0x05);
                    if (fc) { retl(); return; }
                }
            }
        }
    }
    if (from <= 2) { push8(0x81); push16(0xC977); JumpResultDistance_m1x1(); }
    if (from <= 3) { push8(0x81); push16(0xC97B); ShowJumpCountLong_m1x1(); }
    if (from <= 4) { push8(0x81); push16(0xC97F); ShowBonusPointsLong_m1x1(); }
    push8(0x81); push16(0xC983); QueueResultsTiles_m1x1();
    lda8(0x05); wr8(DP(JUMP_BOARD_STEP), 0x05);
    lda8(0x02); wr8(DP(TIMER_B), 0x02);
    lda8(0x3C); wr8(DP(MODE_TIMER), 0x3C);
    lda8(0x01); wr8(ABS(0x1999), 0x01);
    retl();
}
/* $80:E6DB (RTS): the jump contest mode. At one of the broadcast minutes
 * $80:E749 it ends: mode 4 (BG3 at $19, $1905 = $17, $86 cleared) after
 * keeping the best distance and score ($156B set). Otherwise the rank,
 * the course time, the result skip and the step $CD through $80:E739. */
void JumpContestMode_m1x1(void)
{
    ldx8(0x03);
    lda8(rd8(LONG(BCAST_MINUTE)));
    for (;;) {
        cmp8((u8)A, rd8(ABSX(0xE752)));
        if (fz) break;
        X = dec8((u8)X);
        if (fn) {
            push16(0xE733); DrawJumpRank_m1x1();
            push16(0xE736); CheckCourseTime_m1x1();
            push8(0x80); push16(0xE73A); JumpResultSkip_m1x1();
            lda8(rd8(DP(JUMP_BOARD_STEP)));
            lda8(asl8((u8)A));
            X = (u8)A; nz8((u8)X);
            r816_call_pc(0x800000u | rd16(LONGX(0x80E742)));
            return;
        }
    }
    lda8(0x04); wr8(DP(GAME_MODE), 0x04);
    lda8(0x19); wr8(ABS(BG3SC), 0x19);
    lda8(0x01); wr8(ABS(BG34NBA), 0x01);
    lda8(0x17); wr8(ABS(SH_TM), 0x17);
    wr8(DP(UPLOAD_SRC), 0); wr8(DP(0x87), 0);
    X = rd8(ABS(JUMP_RANK)); nz8((u8)X);
    cmp8((u8)X, 0xFF);
    if (!fz) {
        rep(0x20);
        lda16(rd16(ABS(JUMP_COUNT)));
        cmp16(A, rd16(ABS(JUMP_LAST_DIST)));
        if (fc) wr16(ABS(JUMP_LAST_DIST), A);
        push8(0x80); push16(0xE719); ScaleJumpCount_m0x1();
        lda16(rd16(LONGX(0x80F122)));
        and16(0x00FF);
        fc = 0; adc16(rd16(DP(0x0C)));
        cmp16(A, rd16(ABS(JUMP_BEST_SCORE)));
        if (fc) wr16(ABS(JUMP_BEST_SCORE), A);
    }
    sep(0x20);
    ChangeMode_m1x1();
}
/* $81:C56E (RTS): park the jump rider at X $38, ground $B8. */
void ParkJumpRider_m1x1(void)
{
    lda8(0x38); wr8(ABS(RIDER_X), 0x38);
    lda8(0xB8); wr8(ABS(RIDER_Y), 0xB8);
    ret();
}
/* $80:AC0D (RTL): RiderThrottle with DB = $80. */
void RiderThrottleLong_m1x1(void)
{
    push8(DB);
    lda8(0x80); push8(0x80);
    DB = pull8(); nz8(DB);
    push16(0xAC14); RiderThrottle_m1x1();
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:BCF8 (RTL): AddSpeedToX for other banks. */
void AddSpeedToXLong_m1x1(void)
{
    push16(0xBCFA); AddSpeedToX_m1x1();
    retl();
}
/* $80:CD13 (RTL): DrawRiderShadowPair for other banks. */
void DrawRiderShadowPairLong_m1x1(void)
{
    push16(0xCD15); DrawRiderShadowPair_m1x1();
    retl();
}
/* $81:C483 (RTS): the jump contest rider. In the run ($197F = 2) the
 * player rides (on the track, A held: throttle, move, scroll); otherwise
 * (or with $56) it is parked. Then it is drawn with its shadow, and its
 * pose $1961: by $197F off the run ($81:C469), by $56 while landing
 * ($80:CB58, $A8), else by the run frame count $1981 against $81:C473
 * (the run-up poses $81:C46B, walking at speed $0080 with the wheel
 * spark sprites $16C0 cycling through $1987; past the table speed $0500)
 * and the distance step. */
void JumpRiderStep_m1x1(void)
{
    lda8(rd8(ABS(JUMP_RUN_STEP)));
    cmp8((u8)A, 0x02);
    if (!fz) {
        push16(0xC48C); ParkJumpRider_m1x1();
    } else {
        lda8(0x01); wr8(ABS(RIDER_ACTIVE), 0x01);
        lda8(0x80); wr8(ABS(RIDER_INPUT), 0x80);
        ldx8(0x00);
        push8(0x81); push16(0xC49E); RiderThrottleLong_m1x1();
        push8(0x81); push16(0xC4A2); AddSpeedToXLong_m1x1();
        ldy8(0x00);
        push16(0xC4A7); ScrollJumpRider_m1x1();
    }
    lda8(rd8(DP(0x56)));
    if (!fz) { push16(0xC4AE); ParkJumpRider_m1x1(); }
    ldx8(0x00);
    wr8(ABS(RANK_ORDER), 0);
    push8(0x81); push16(0xC4B7); DrawAllRidersLong_m1x1();
    ldy8(0x00);
    push16(0xC4BC); SlotRiderShadow_m1x1();
    lda8(rd8(ABS(JUMP_RUN_STEP)));
    cmp8((u8)A, 0x02);
    if (!fz) {
        X = rd8(ABS(JUMP_RUN_STEP)); nz8((u8)X);
        lda8(rd8(ABSX(0xC469)));
        wr8(ABS(SPRITE_FRAME), (u8)A);
        ret();
        return;
    }
    lda8(rd8(DP(0x56)));
    if (!fz) {
        cmp8((u8)A, 0x01);
        if (!fz) {
            lda8(dec8((u8)A));
            ora8(0x10);
            wr8(DP(0xA8), (u8)A);
            X = (u8)A; nz8((u8)X);
            lda8(rd8(LONGX(0x80CB61)));
        }
        wr8(ABS(SPRITE_FRAME), (u8)A);
        ret();
        return;
    }
    rep(0x20);
    wr16(ABS(JUMP_RUN_FRAMES), inc16(rd16(ABS(JUMP_RUN_FRAMES))));
    ldx8(0x00);
    lda16(rd16(ABS(JUMP_RUN_FRAMES)));
    int past = 0;
    for (;;) {
        cmp16(A, rd16(ABSX(0xC473)));
        if (!fc) break;
        X = inc8((u8)X);
        X = inc8((u8)X);
        cmp8((u8)X, 0x10);
        if (fz) { past = 1; break; }
    }
    if (past) {
        sep(0x20);
        lda8(0x00); wr8(ABS(RIDER_SPEED_LO), 0x00);
        lda8(0x05); wr8(ABS(RIDER_SPEED_HI), 0x05);
    } else {
        sep(0x20);
        lda8((u8)X);
        lda8(lsr8((u8)A));
        X = (u8)A; nz8((u8)X);
        if (fz) { lda8(rd8(DP(FRAME_COUNT))); and8(0x01); }
        else lda8(rd8(ABSX(0xC46B)));
        wr8(ABS(SPRITE_FRAME), (u8)A);
        lda8(0x80); wr8(ABS(RIDER_SPEED_LO), 0x80);
        lda8(0x00); wr8(ABS(RIDER_SPEED_HI), 0x00);
        lda8(rd8(ABS(RIDER_PAGE)));
        cmp8((u8)A, 0x02);
        if (!fc) {
            ldx8(0x00);
            Y = X; nz8((u8)Y);
            lda8(rd8(ABS(RIDER_X)));
            push8(0x81); push16(0xC52C); DrawRiderShadowPairLong_m1x1();
            wr8(ABS(0x1987), inc8(rd8(ABS(0x1987))));
            lda8(rd8(ABS(0x1987)));
            cmp8((u8)A, 0x01);
            if (fz) {
                lda8(0xF0); wr8(ABSY(OAM_BUF + 0xC2), 0xF0);
                lda8(0xE0); wr8(ABSY(OAM_BUF + 0xC6), 0xE0);
            } else {
                cmp8((u8)A, 0x02);
                if (fz) {
                    lda8(0xF0); wr8(ABSY(OAM_BUF + 0xC1), 0xF0); wr8(ABSY(OAM_BUF + 0xC5), 0xF0);
                } else wr8(ABS(0x1987), 0);
            }
            lda8(0x2F); wr8(ABSY(OAM_BUF + 0xC3), 0x2F); wr8(ABSY(OAM_BUF + 0xC7), 0x2F);
        }
    }
    push16(0xC56C); DispatchByDistance_m1x1();
    ret();
}
/* $81:C2D9 (RTS): the jump run's start: once racing ($92), the run length
 * $1985 for this entry ($81:C2C9 by $7C, +8 with the debug flag), speed
 * $0500 and the next run step ($197F). Then the crowd and the rider. */
void JumpRunStart_m1x1(void)
{
    rep(0x20);
    lda16(rd16(DP(RACE_STARTED)));
    if (!fz) {
        wr16(DP(0x00), 0);
        lda16(rd16(LONG(STADIUM_SELECT)));
        if (!fz) { lda16(0x0008); wr16(DP(0x00), 0x0008); }
        lda16(rd16(DP(COURSE_SLOT)));
        and16(0x00FF);
        lda16(asl16(A));
        fc = 0; adc16(rd16(DP(0x00)));
        X = (u8)A; nz8((u8)X);
        lda16(rd16(LONGX(0x81C2C9)));
        wr16(ABS(0x1985), A);
        sep(0x20);
        lda8(0x00); wr8(ABS(RIDER_SPEED_LO), 0x00);
        lda8(0x05); wr8(ABS(RIDER_SPEED_HI), 0x05);
        wr8(ABS(JUMP_RUN_STEP), inc8(rd8(ABS(JUMP_RUN_STEP))));
    }
    sep(0x20);
    push16(0xC310); DrawJumpCrowd_m1x1();
    push16(0xC313); JumpRiderStep_m1x1();
    ret();
}
/* $81:C5AA (RTS): the player moves and the screen follows. */
void MoveAndScroll_m1x1(void)
{
    ldx8(0x00);
    push8(0x81); push16(0xC5AF); AddSpeedToXLong_m1x1();
    push8(0x81); push16(0xC5B3); ScrollByPlayerSpeedLong_m1x1();
    ret();
}
/* $81:C331 (RTS): the jump run. Until the run length $1985 the track
 * streams ($81:8A4D), the rider moves, the crowd and wheels animate.
 * At the end, unless the time is short ($88 < $0654), the landing
 * banner ($81:C315, $0207 at VRAM $194D), speed $0080, $56 = 3, sound
 * $45, $198A = $30 and the next step. Then the rider. */
void JumpRun_m1x1(void)
{
    rep(0x20);
    lda16(rd16(DP(SCROLL_X)));
    cmp16(A, rd16(ABS(0x1985)));
    if (!fc) {
        sep(0x20);
        push8(0x81); push16(0xC383); LoadRaceColumn_m1x1();
        push16(0xC386); MoveAndScroll_m1x1();
        push16(0xC389); DrawJumpCrowd_m1x1();
        push16(0xC38C); SpinWheels8_m1x1();
    } else {
        lda16(rd16(DP(MODE_TIMER)));
        cmp16(A, 0x0654);
        if (fc) {
            wr16(ABS(JUMP_RUN_FRAMES), 0);
            wr16(ABS(0x1988), 0);
            lda16(0x194D); wr16(DP(0x00), 0x194D);
            lda16(0xC315); wr16(DP(0x02), 0xC315);
            lda16(0x0207); wr16(DP(0x10), 0x0207);
            sep(0x20);
            lda8(0x81); wr8(DP(0x04), 0x81);
            push8(0x81); push16(0xC35F); QueueTileRowsLong_m1x1();
            lda8(0x80); wr8(ABS(RIDER_SPEED_LO), 0x80);
            lda8(0x00); wr8(ABS(RIDER_SPEED_HI), 0x00);
            lda8(0x03); wr8(DP(0x56), 0x03);
            lda8(0x45);
            push8(0x81); push16(0xC373); QueueSoundB_m1x1();
            lda8(0x30); wr8(ABS(ENGINE_PITCH), 0x30);
            wr8(ABS(JUMP_RUN_STEP), inc8(rd8(ABS(JUMP_RUN_STEP))));
        }
    }
    sep(0x20);
    push16(0xC391); JumpRiderStep_m1x1();
    ret();
}
/* $80:8F0E (RTL): ChangeMode for other banks. */
void ChangeModeLong_m1x1(void)
{
    push16(0x8F10); ChangeMode_m1x1();
    retl();
}
/* $81:C3B1 (RTS): leave the jump contest for mode 3. */
void LeaveJumpContest_m1x1(void)
{
    lda8(0x03); wr8(DP(GAME_MODE), 0x03);
    push8(0x81); push16(0xC3B8); ChangeModeLong_m1x1();
    ret();
}
/* $81:C29A (RTL): the jump contest's run steps: Start with the debug flag
 * leaves; otherwise $88 counts up and the run step $197F runs through
 * $81:C2C1 (start, run, ...), then the banner. */
void JumpRunSteps_m1x1(void)
{
    lda8(rd8(LONG(DEBUG_ENABLED)));
    if (!fz) {
        lda8(rd8(DP(P1_PRESS_HI)));
        and8(0x10);
        if (!fz) { push16(0xC2A8); LeaveJumpContest_m1x1(); retl(); return; }
    }
    push8(DB);
    push8(0x81);
    DB = pull8(); nz8(DB);
    rep(0x20);
    wr16(DP(MODE_TIMER), inc16(rd16(DP(MODE_TIMER))));
    sep(0x20);
    lda8(rd8(ABS(JUMP_RUN_STEP)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    push16(0xC2BB);
    r816_call_pc(0x810000u | rd16(LONGX(0x81C2C1)));
    push16(0xC2BE); QueueBlinkBanner_m1x1();
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:FE27 (RTS): A + 1, wrapping at $40. */
void NextColumn_m1x1(void)
{
    fc = 0; adc8(0x01);
    cmp8((u8)A, 0x40);
    if (fz) lda8(0x00);
    ret();
}
/* $80:FE23 (RTL): NextColumn for other banks. */
void NextColumnLong_m1x1(void)
{
    push16(0xFE25); NextColumn_m1x1();
    retl();
}
/* $81:8A42 (RTL, X = row type): the row's piece list ($05) from
 * $81:8010 / $81:8040 (DB = $81). */
void RaceRowList_m1x1(void)
{
    lda8(rd8(ABSX(0x8010))); wr8(DP(0x05), (u8)A);
    lda8(rd8(ABSX(0x8040))); wr8(DP(0x06), (u8)A);
    retl();
}
/* $81:8A4D (RTL): load the race course one column ahead (DB = $81). The
 * expanded course $7F:4000 ([$07]) holds the lap count at entry 0 ($0C in
 * a race, $7D clear; $0928 + 1, $AE cleared) and then row entries: with a
 * $81:8070 bit a row type from $81:8A30 by the low nibble, else the low
 * seven bits. Each time the builder column $0279 reaches the player
 * column $0266 the row list ($81:8A42) gives the next column: blanked,
 * its entry recorded at $7F:6000 + $AE, filled (CopyTrackColumn), on the
 * finish row ($09) the lap digits ($81:8A3D by $0929, then the lap $0928
 * as 1 and its units) at $7F:0026, and queued for VRAM at $0277 - $20
 * (FillLaneColumn; $0277 wraps at $20 with $0278 ^ 4). At the end of a
 * finish row the next lap starts ($03AC cleared, $0928 + 1, $093F - 1,
 * $098A = 1 on the last; the course restarts, $B0 = $AE). */
void LoadRaceColumn_m1x1(void)
{
    push8(DB);
    lda8(0x81); push8(0x81);
    DB = pull8(); nz8(DB);
    lda8(rd8(DP(COURSE_SLOT)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    lda8(0x00); wr8(DP(0x07), 0x00);
    lda8(0x40); wr8(DP(0x08), 0x40);
    lda8(0x7F); wr8(DP(0x09), 0x7F);
    lda8(rd8(ABS(0x0279)));
    cmp8((u8)A, rd8(ABS(RIDER_COLUMN)));
    if (!fz) goto out;
    lda8(rd8(ABS(0x027E)));
    if (!fz) goto column;
next_entry:
    WAIT_TICK();
    rep(0x10);
    Y = rd16(ABS(TRACK_READ_POS)); nz16(Y);
    if (fz) {
        lda8(rd8(ILDPY(0x07)));
        wr8(ABS(LAPS_LEFT), (u8)A);
        wr8(ABS(LAP), inc8(rd8(ABS(LAP))));
        wr8(DP(COURSE_COLUMN), 0);
        wr8(DP(0xAF), 0);
        lda8(rd8(DP(FINAL_RACE)));
        if (fz) { lda8(0x0C); wr8(ABS(LAPS_LEFT), 0x0C); }
    }
read_entry:
    rep(0x20);
    wr16(ABS(TRACK_READ_POS), inc16(rd16(ABS(TRACK_READ_POS))));
    sep(0x20);
    if (fx) { Y = rd8(ABS(TRACK_READ_POS)); nz8((u8)Y); }
    else { Y = rd16(ABS(TRACK_READ_POS)); nz16(Y); }
    lda8(rd8(ILDPY(0x07)));
    if (fz) goto out;
    sep(0x10);
    bit8(rd8(ABS(0x8070)), 0);
    if (!fz) {
        and8(0x0F);
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(0x8A30)));
        wr8(ABS(ROW_TYPE), (u8)A);
    } else {
        sep(0x10);
        ldx8(0x01);
        and8(0x7F);
        wr8(ABS(ROW_TYPE), (u8)A);
    }
column:
    X = rd8(ABS(ROW_TYPE)); nz8((u8)X);
    push8(0x81); push16(0x8AC1); RaceRowList_m1x1();
    Y = rd8(ABS(0x027E)); nz8((u8)Y);
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x05)) + Y) & 0xFFFFFF));
    if (fz) {
        wr8(ABS(0x027E), (u8)A);
        lda8(rd8(ABS(ROW_TYPE)));
        cmp8((u8)A, 0x09);
        if (!fz) goto next_entry;
        wr8(ABS(FINISH_CROSSED), 0);
        wr8(ABS(LAP), inc8(rd8(ABS(LAP))));
        wr8(ABS(LAPS_LEFT), dec8(rd8(ABS(LAPS_LEFT))));
        if (fz) { lda8(0x01); wr8(ABS(0x098A), 0x01); }
        lda8(0x00);
        wr8(ABS(TRACK_READ_POS), 0);
        wr8(ABS(0x027C), 0);
        wr8(ABS(0x0929), 0);
        rep(0x20);
        lda16(rd16(DP(COURSE_COLUMN)));
        wr16(DP(0xB0), A);
        wr16(DP(COURSE_COLUMN), 0);
        sep(0x20);
        goto read_entry;
    }
    push8(0x81); push16(0x8ACF); BlankQueueRowPal2_m1x1();
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x05)) + Y) & 0xFFFFFF));
    push8((u8)A);
    rep(0x30);
    X = rd16(DP(COURSE_COLUMN)); nz16(X);
    lda16(rd16(ABS(TRACK_READ_POS)));
    wr16(LONGX(TRACK_COL_INDEX), A);
    wr16(DP(COURSE_COLUMN), inc16(rd16(DP(COURSE_COLUMN))));
    wr16(DP(COURSE_COLUMN), inc16(rd16(DP(COURSE_COLUMN))));
    sep(0x30);
    X = pull8(); nz8((u8)X);
    push8(0x81); push16(0x8AE8); CopyTrackColumn_m1x1();
    Y = inc8((u8)Y);
    Y = inc8((u8)Y);
    wr8(ABS(0x027E), (u8)Y);
    lda8(rd8(ABS(ROW_TYPE)));
    cmp8((u8)A, 0x09);
    if (fz) {
        lda8(rd8(ABS(LAPS_LEFT)));
        cmp8((u8)A, 0x01);
        if (!fz) {
            cmp8((u8)Y, 0x4A);
            if (fc) {
                cmp8((u8)Y, 0xB0);
                if (!fc) {
                    X = rd8(ABS(0x0929)); nz8((u8)X);
                    lda8(rd8(ABSX(0x8A3D)));
                    wr8(LONG(VQ0_DATA + 0x18), (u8)A);
                    wr8(ABS(0x0929), inc8(rd8(ABS(0x0929))));
                    cmp8((u8)X, 0x04);
                    if (fc) {
                        if (fz) {
                            lda8(rd8(ABS(LAP)));
                            cmp8((u8)A, 0x0A);
                            if (fc) { lda8(0x01); wr8(LONG(VQ0_DATA + 0x18), 0x01); }
                        } else {
                            lda8(rd8(ABS(LAP)));
                            cmp8((u8)A, 0x0A);
                            if (fc) { fc = 1; sbc8(0x0A); }
                            wr8(LONG(VQ0_DATA + 0x18), (u8)A);
                        }
                    }
                }
            }
        }
    }
    lda8(0x81); wr8(LONG(VQ0_VMAIN), 0x81);
    lda8(0x01); wr8(LONG(VQ0_DMAP), 0x01);
    lda8(0x18); wr8(LONG(VQ0_BBAD), 0x18);
    lda8(0x02);
    fc = 0; adc8(0x0C);
    wr8(LONG(VQ0_SRC), (u8)A);
    lda8(0x00); wr8(LONG(VQ0_SRC_HI), 0);
    lda8(0x7F); wr8(LONG(VQ0_SRC_BANK), 0x7F);
    rep(0x20);
    lda16(0x0002);
    fc = 0; adc16(0x0028);
    wr16(LONG(VQ_END), A);
    wr16(LONG(VQ0_NEXT), A);
    lda16(rd16(ABS(TRACK_VRAM_COL)));
    fc = 1; sbc16(0x0020);
    wr16(LONG(VQ0_VMADD), A);
    lda16(0x001C); wr16(LONG(VQ0_SIZE), 0x001C);
    sep(0x20);
    lda8(rd8(ABS(TRACK_ROW_PTR)));
    wr8(DP(0x0F), (u8)A);
    push8(0x81); push16(0x8B88); FillLaneColumn_m1x1();
    lda8(rd8(ABS(TRACK_ROW_PTR)));
    push8(0x81); push16(0x8B8F); NextColumnLong_m1x1();
    wr8(ABS(TRACK_ROW_PTR), (u8)A);
    wr8(ABS(TRACK_VRAM_COL), inc8(rd8(ABS(TRACK_VRAM_COL))));
    lda8(rd8(ABS(TRACK_VRAM_COL)));
    cmp8((u8)A, 0x20);
    if (fz) {
        lda8(0x00); wr8(ABS(TRACK_VRAM_COL), 0);
        lda8(rd8(ABS(TRACK_VRAM_HALF)));
        eor8(0x04);
        wr8(ABS(TRACK_VRAM_HALF), (u8)A);
    }
    lda8(rd8(ABS(RIDER_COLUMN)));
    push8(0x81); push16(0x8BB0); NextColumnLong_m1x1();
    wr8(ABS(0x0279), (u8)A);
    DB = pull8(); nz8(DB);
    retl();
    return;
out:
    sep(0x10);
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:DBFA (RTS, X = row type): the row piece list ($05) from
 * $80:D2FB / $80:D304. */
void CourseRowList_m1x1(void)
{
    lda8(rd8(ABSX(0xD304))); wr8(DP(0x05), (u8)A);
    lda8(rd8(ABSX(0xD30D))); wr8(DP(0x06), (u8)A);
    ret();
}
/* $80:DA80 (RTS): build the course one column ahead. The course script
 * ($07 from $80:D270 / $80:D272 by the header flag $81:800C) starts with
 * the scenery byte $09C9. Each time the builder column $0279 reaches the
 * player column $0266: the next script entry (0 = stop, 9 = loop to
 * entry 1 as row type 1, $80+ = that many empty rows, else a row type
 * $027D) and its row list ($80:DBF1) give the column pieces; a column
 * is blanked, filled (CopyPieceColumnPal2) and queued for VRAM at $0277 + $07E0 (row
 * pointer $0276, column $0277 wrapping $80 -> $60 with $0278 ^ 4), and
 * the builder column steps. */
void BuildCourseColumn_m1x1(void)
{
    lda8(rd8(LONG(STADIUM_SELECT)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0xD279))); wr8(DP(0x07), (u8)A);
    lda8(rd8(ABSX(0xD27B))); wr8(DP(0x08), (u8)A);
    Y = rd8(ABS(TRACK_READ_POS)); nz8((u8)Y);
    if (fz) {
        lda8(course_list4(0x07));
        wr8(ABS(0x09C9), (u8)A);
        Y = inc8((u8)Y);
        wr8(ABS(TRACK_READ_POS), (u8)Y);
    }
    lda8(rd8(ABS(0x0279)));
    cmp8((u8)A, rd8(ABS(RIDER_COLUMN)));
    if (!fz) { ret(); return; }
    int step;   /* 0 = next entry, 1 = counting empty rows, 2 = same row */
    lda8(rd8(ABS(0x027A)));
    if (!fz) step = 1;
    else {
        lda8(rd8(ABS(0x027E)));
        step = fz ? 0 : 2;
    }
    for (;;) {
        WAIT_TICK();
        if (step == 0) {
            Y = rd8(ABS(TRACK_READ_POS)); nz8((u8)Y);
            wr8(ABS(TRACK_READ_POS), inc8(rd8(ABS(TRACK_READ_POS))));
            lda8(course_list4(0x07));
            if (fz) { ret(); return; }
            cmp8((u8)A, 0x09);
            if (fz) {
                lda8(0x01);
                wr8(ABS(TRACK_READ_POS), 0x01);
            }
            cmp8((u8)A, 0x80);
            if (fc) {
                and8(0x7F);
                wr8(ABS(0x027A), (u8)A);
                step = 1;
            } else {
                wr8(ABS(ROW_TYPE), (u8)A);
                step = 2;
            }
        }
        if (step == 1) {
            wr8(ABS(0x027A), dec8(rd8(ABS(0x027A))));
            lda8(0x00);
            wr8(ABS(BUILD_COLUMN), 0);
            wr8(ABS(0x027E), 0);
            wr8(ABS(ROW_TYPE), 0);
        }
        /* L80DBE5 */
        X = rd8(ABS(ROW_TYPE)); nz8((u8)X);
        push16(0xDAE1); CourseRowList_m1x1();
        Y = rd8(ABS(0x027E)); nz8((u8)Y);
        lda8(course_list4(0x05));
        if (!fz) break;
        wr8(ABS(0x027E), (u8)A);
        wr8(ABS(BUILD_COLUMN), (u8)A);
        step = 0;
    }
    push16(0xDAEE); BlankQueueRow_m1x1();
    lda8(course_list4(0x05));
    X = (u8)A; nz8((u8)X);
    push16(0xDAF4); CopyPieceColumnPal2_m1x1();
    Y = inc8((u8)Y);
    Y = inc8((u8)Y);
    wr8(ABS(0x027E), (u8)Y);
    lda8(0x81); wr8(LONG(VQ0_VMAIN), 0x81);
    lda8(0x01); wr8(LONG(VQ0_DMAP), 0x01);
    lda8(0x18); wr8(LONG(VQ0_BBAD), 0x18);
    lda8(rd8(LONG(VQ_END)));
    fc = 0; adc8(0x0C);
    wr8(LONG(VQ0_SRC), (u8)A);
    lda8(0x00); wr8(LONG(VQ0_SRC_HI), 0);
    lda8(0x7F); wr8(LONG(VQ0_SRC_BANK), 0x7F);
    rep(0x20);
    lda16(rd16(LONG(VQ_END)));
    fc = 0; adc16(0x002A);
    wr16(LONG(VQ_END), A);
    wr16(LONG(VQ0_NEXT), A);
    lda16(rd16(ABS(TRACK_VRAM_COL)));
    fc = 0; adc16(0x07E0);
    wr16(LONG(VQ0_VMADD), A);
    lda16(0x001E); wr16(LONG(VQ0_SIZE), 0x001E);
    sep(0x20);
    lda8(rd8(ABS(TRACK_ROW_PTR)));
    wr8(DP(0x0F), (u8)A);
    push16(0xDB50); CourseColumn_m1x1();
    lda8(rd8(ABS(TRACK_ROW_PTR)));
    push16(0xDB56); NextColumn_m1x1();
    wr8(ABS(TRACK_ROW_PTR), (u8)A);
    wr8(ABS(TRACK_VRAM_COL), inc8(rd8(ABS(TRACK_VRAM_COL))));
    lda8(rd8(ABS(TRACK_VRAM_COL)));
    cmp8((u8)A, 0x80);
    if (fz) {
        lda8(0x60); wr8(ABS(TRACK_VRAM_COL), 0x60);
        lda8(rd8(ABS(TRACK_VRAM_HALF)));
        eor8(0x04);
        wr8(ABS(TRACK_VRAM_HALF), (u8)A);
    }
    lda8(rd8(ABS(RIDER_COLUMN)));
    push16(0xDB76); NextColumn_m1x1();
    wr8(ABS(0x0279), (u8)A);
    ret();
}
/* $80:F75B RunRiderPieces (RTS, X = rider): the rider's piece script.
 * Its distance $024E advances by the speed $0186. With no script running
 * ($0176 = 0) a piece type $021E below $1A starts script type + 1 (a $1A+
 * piece clears bonus tiles and, in jump state 3 in lanes 0-2, starts $0C
 * + 1; otherwise the player's $1979 clears). In a free run ($7D set) a
 * rider outside X $00B0-$026F instead drops out of the script and its
 * jump (lane height $80:F8CD). Then $0256 = 0 and $024E = $018E - 1, and
 * the script ($80:F536 / $80:F54F) runs pairs (distance, action) up to
 * $024E: an action with a $80:F6DE bit sets the angle ($01E6, $01A6 from
 * $80:F6D3) on the ground; one with a $80:F6DD bit sets $0226 / $023E;
 * others go through PieceAction ($80:F850). At the $FF end the script
 * stops ($0176, $01A6 cleared, $01FE cleared on the ground outside jump
 * state 1, state 2 -> 3) and $08BC clears. */
void RunRiderPieces_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_STEP)));
    fc = 0; adc8(rd8(ABSX(RIDER_DISTANCE)));
    wr8(ABSX(RIDER_DISTANCE), (u8)A);
    lda8(rd8(ABSX(RIDER_SCRIPT)));
    if (fz) {
        lda8(rd8(ABSX(RIDER_TILE)));
        cmp8((u8)A, 0x1A);
        if (fc) {
            push8(0x80); push16(0xF774); ClearOnBonusTiles_m1x1();
            int start = 0;
            lda8(rd8(ABSX(RIDER_JUMP)));
            cmp8((u8)A, 0x03);
            if (fz) {
                lda8(rd8(ABSX(RIDER_LANE_NUM)));
                cmp8((u8)A, 0x03);
                if (!fc) {
                    lda8(0x0C);
                    wr8(ABSX(RIDER_TILE), 0x0C);
                    start = 1;
                }
            }
            if (!start) {
                cmp8((u8)X, 0x00);
                if (fz) wr8(ABS(0x1979), 0);
                ret();
                return;
            }
        }
        wr8(ABSX(RIDER_SCRIPT), (u8)A);
        wr8(ABSX(RIDER_SCRIPT), inc8(rd8(ABSX(RIDER_SCRIPT))));
        lda8(rd8(DP(FINAL_RACE)));
        if (!fz) {
            lda8(rd8(ABSX(RIDER_PAGE)));
            A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
            lda8(rd8(ABSX(RIDER_X)));
            rep(0x20);
            int out = 0;
            cmp16(A, 0x00B0);
            if (!fc) out = 1;
            else {
                cmp16(A, 0x0270);
                out = fc;
            }
            if (out) {
                sep(0x20);
                wr8(ABSX(RIDER_PIECE_ACTION), 0);
                wr8(ABSX(RIDER_POSE), 0);
                wr8(ABSX(0x0981), 0);
                wr8(ABSX(RIDER_SCRIPT), 0);
                wr8(ABSX(RIDER_HEIGHT), 0);
                Y = rd8(ABSX(RIDER_LANE_NUM)); nz8((u8)Y);
                lda8(rd8(ABSY(0xF89A)));
                wr8(ABSX(RIDER_LANE), (u8)A);
                wr8(ABSX(RIDER_TIMER), 0);
                wr8(ABSX(0x199E), 0);
                wr8(ABSX(RIDER_STEER), 0);
                ret();
                return;
            }
        }
        sep(0x20);
        lda8(0x00);
        wr8(ABSX(0x0256), 0);
        lda8(rd8(ABSX(RIDER_WHEEL)));
        wr8(ABSX(RIDER_DISTANCE), (u8)A);
        wr8(ABSX(RIDER_DISTANCE), dec8(rd8(ABSX(RIDER_DISTANCE))));
    }
    for (;;) {
        /* L80F816 */
        Y = rd8(ABSX(RIDER_SCRIPT)); nz8((u8)Y);
        lda8(rd8(ABSY(0xF503))); wr8(DP(0x0A), (u8)A);
        lda8(rd8(ABSY(0xF51C))); wr8(DP(0x0B), (u8)A);
        Y = rd8(ABSX(0x0256)); nz8((u8)Y);
        lda8(script_byte4());
        cmp8((u8)A, 0xFF);
        if (fz) break;
        wr8(DP(0x0F), (u8)A);
        cmp8((u8)A, rd8(ABSX(RIDER_DISTANCE)));
        if (!fz && fc) { ret(); return; }
        Y = inc8((u8)Y);
        lda8(script_byte4());
        bit8(rd8(ABS(0xF6AB)), 0);
        if (!fz) {
            /* L80F85F */
            lda8(rd8(ABSX(RIDER_AIR)));
            ora8(rd8(ABSX(RIDER_BUMPED)));
            if (fz) {
                lda8(script_byte4());
                and8(0x0F);
                wr8(DP(0x00), (u8)A);
                lda8(rd8(ABSX(RIDER_JUMP)));
                cmp8((u8)A, 0x01);
                if (!fz) {
                    lda8(rd8(DP(0x00)));
                    wr8(ABSX(RIDER_LEAN), (u8)A);
                    lda8(rd8(ABSX(RIDER_SCRIPT)));
                    cmp8((u8)A, 0x03);
                    if (!fz) {
                        lda8(rd8(ABSX(RIDER_LEAN)));
                        fc = 1; sbc8(0x02);
                        Y = (u8)A; nz8((u8)Y);
                        lda8(rd8(ABSY(0xF6A0)));
                        wr8(ABSX(RIDER_POSE), (u8)A);
                    }
                }
            }
        } else {
            bit8(rd8(ABS(0xF6AA)), 0);
            if (fz) {
                push16(0xF814); PieceAction_m1x1();
                wr8(ABSX(0x0256), inc8(rd8(ABSX(0x0256))));
                wr8(ABSX(0x0256), inc8(rd8(ABSX(0x0256))));
                if (!fz) continue;
                PieceAction_m1x1();
                return;
            }
            /* L80F8C1 */
            and8(0x1F);
            wr8(ABSX(RIDER_PIECE_ACTION), (u8)A);
            lda8(rd8(DP(0x0F)));
            wr8(ABSX(0x023E), (u8)A);
            if (fn) {
                /* falls into the table bytes at $80:F8CD, decoded as code,
                 * and on into RiderPieceAt */
                and8(rd8((((u32)DB << 16) + rd16(DP(0x32))) & 0xFFFFFF));
                { u32 ea = DP(0x1A); wr8(ea, rol8(rd8(ea))); }
                lda8(inc8((u8)A));
                { u32 ea = ABS(0xA4BC); wr8(ea, asl8(rd8(ea))); }
                push8(getP());
                RiderPieceAt_m1x1();
                return;
            }
        }
        /* L80F88D */
        wr8(ABSX(0x0256), inc8(rd8(ABSX(0x0256))));
        wr8(ABSX(0x0256), inc8(rd8(ABSX(0x0256))));
        if (fz) break;
    }
    /* L80F898 */
    lda8(0x00);
    wr8(ABSX(RIDER_SCRIPT), 0);
    wr8(ABSX(RIDER_POSE), 0);
    lda8(rd8(ABSX(RIDER_LANDING)));
    if (fz) {
        lda8(rd8(ABSX(RIDER_JUMP)));
        cmp8((u8)A, 0x01);
        if (!fz) {
            lda8(0x00);
            wr8(ABSX(RIDER_HEIGHT), 0);
            lda8(rd8(ABSX(RIDER_JUMP)));
            cmp8((u8)A, 0x02);
            if (fz) wr8(ABSX(RIDER_JUMP), inc8(rd8(ABSX(RIDER_JUMP))));
        }
    }
    lda8(0x00);
    wr8(ABSX(RIDER_ON_RAMP), 0);
    ret();
}
/* $80:ABD0 (RTS): while racing ($92), RidersThrottle for all eight riders;
 * otherwise only its closing check (unpaused, the player holding A/B and
 * $092D, $84 clear: $092D = 1). */
void ThrottleAll_m1x1(void)
{
    lda8(rd8(DP(RACE_STARTED)));
    if (!fz) {
        ldx8(0x07);
        RidersThrottle_m1x1();
        return;
    }
    lda8(rd8(DP(0x54)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(RIDER_INPUT)));
    and8(0xC0);
    if (fz) { ret(); return; }
    lda8(rd8(ABS(GAUGE_ON)));
    ora8(rd8(DP(RACE_END)));
    if (!fz) { ret(); return; }
    lda8(0x01); wr8(ABS(GAUGE_ON), 0x01);
    ret();
}
/* $80:BB4F (RTS, X = rider): the rider's state step $01D6 through the
 * table $80:BB1F / $80:BB25. */
void RiderStateStep_m1x1(void)
{
    Y = rd8(ABSX(RIDER_TIMER)); nz8((u8)Y);
    lda8(rd8(ABSY(0xBB28))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSY(0xBB2E))); wr8(DP(0x01), (u8)A);
    if (!r816_call_pc(0x800000u | rd16(LONG(0x000000))))
        r816_unknown_target(0x80BB53, rd16(LONG(0x000000)));
}
/* $80:BB43 (RTS): for riders 7-0 the state step and the shadow height. */
void RiderStepAll_m1x1(void)
{
    ldx8(0x07);
    do {
        push16(0xBB47); RiderStateStep_m1x1();
        push16(0xBB4A); RiderShadowY_m1x1();
        X = dec8((u8)X);
    } while (!fn);
    ret();
}
/* $80:CA9C (RTS): sort the riders by position, then draw all eight. */
void SortAndDrawRiders_m1x1(void)
{
    push16(0xCA9E); SortRidersByPosition_m1x1();
    ldx8(0x07);
    DrawAllRiders_m1x1();
}
/* $80:F722 (RTS): SpinWheels for all eight riders. */
void SpinAllWheels_m1x1(void)
{
    ldx8(0x07);
    SpinWheels_m1x1();
}
/* $80:F753 (RTS): the piece scripts of riders 7-1, then the player's. */
void RunAllRiderPieces_m1x1(void)
{
    ldx8(0x07);
    do {
        push16(0xF757); RunRiderPieces_m1x1();
        X = dec8((u8)X);
    } while (!fz);
    RunRiderPieces_m1x1();
}
/* $80:F8FE (RTS, X = last rider): riders X-0 on the ground get their
 * height $01B6 from GroundHeightAbove. */
void RiderGroundHeightsFrom_m1x1(void)
{
    do {
        lda8(rd8(ABSX(RIDER_AIR)));
        if (fz) {
            push16(0xF905); GroundHeightAbove_m1x1();
            wr8(ABSX(RIDER_Y), (u8)A);
        }
        X = dec8((u8)X);
    } while (!fn);
    ret();
}
/* $80:F8FC (RTS): the same for all eight riders. */
void RiderGroundHeights_m1x1(void)
{
    ldx8(0x07);
    RiderGroundHeightsFrom_m1x1();
}
/* $80:FBC2 (RTS): for riders 7-0 with a pending piece action $0226, run
 * it (PieceAction). */
void RunPieceActions_m1x1(void)
{
    ldx8(0x07);
    do {
        lda8(rd8(ABSX(RIDER_PIECE_ACTION)));
        if (!fz) { push16(0xFBCB); PieceAction_m1x1(); }
        X = dec8((u8)X);
    } while (!fn);
    ret();
}
/* $80:F2FA (RTS): TurboMeter. */
void TurboMeterJmp_m1x1(void)
{
    TurboMeter_m1x1();
}
/* $80:C696 (RTL): the best time at VRAM $18D7 (palette $24) and, before
 * the race end ($84 clear), the race clock restarted; X and Y kept. */
void ShowCourseBest_m1x1(void)
{
    push8((u8)X); push8((u8)Y);
    lda8(0xD7); wr8(DP(0x00), 0xD7);
    lda8(0x18); wr8(DP(0x01), 0x18);
    lda8(0x24); wr8(DP(0x0F), 0x24);
    push16(0xC708); QueueBestTime_m1x1();
    lda8(rd8(DP(RACE_END)));
    if (fz) {
        wr8(ABS(RACE_MIN), 0);
        wr8(ABS(RACE_SEC), 0);
        wr8(ABS(RACE_FRAMES), 0);
    }
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    retl();
}
/* $80:EE82 (RTS, X = row type): the bonus course row piece list ($05)
 * from $80:E00D / $80:E01E. */
void BonusRowList_m1x1(void)
{
    lda8(rd8(ABSX(0xE016))); wr8(DP(0x05), (u8)A);
    lda8(rd8(ABSX(0xE027))); wr8(DP(0x06), (u8)A);
    ret();
}
/* $80:ECEF (RTS): build the bonus course one column ahead, as
 * BuildCourseColumn but from the long script ([$07] from $81:CAAC /
 * $81:CAB4 / $81:CABC by 4 * $81:800C + $7C), empty-row counts in the
 * low five bits, row type 3 counting $97, the row lists $80:EEB0 and the
 * plain column copy (BlankQueueRowB, CopyPieceColumn, CourseColumnB). */
void BuildBonusColumn_m1x1(void)
{
    lda8(rd8(LONG(STADIUM_SELECT)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    wr8(DP(0x00), (u8)A);
    lda8(rd8(ABS(COURSE_SLOT)));
    fc = 0; adc8(rd8(DP(0x00)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(LONGX(0x81CAAC))); wr8(DP(0x07), (u8)A);
    lda8(rd8(LONGX(0x81CAB4))); wr8(DP(0x08), (u8)A);
    lda8(rd8(LONGX(0x81CABC))); wr8(DP(0x09), (u8)A);
    Y = rd8(ABS(TRACK_READ_POS)); nz8((u8)Y);
    if (fz) {
        lda8(rd8(ILDPY(0x07)));
        wr8(ABS(0x09C9), (u8)A);
        Y = inc8((u8)Y);
        wr8(ABS(TRACK_READ_POS), (u8)Y);
    }
    lda8(rd8(ABS(0x0279)));
    cmp8((u8)A, rd8(ABS(RIDER_COLUMN)));
    if (!fz) { ret(); return; }
    int step;   /* 0 = next entry, 1 = counting empty rows, 2 = same row */
    lda8(rd8(ABS(0x027A)));
    if (!fz) step = 1;
    else {
        lda8(rd8(ABS(0x027E)));
        step = fz ? 0 : 2;
    }
    for (;;) {
        WAIT_TICK();
        if (step == 0) {
            Y = rd8(ABS(TRACK_READ_POS)); nz8((u8)Y);
            wr8(ABS(TRACK_READ_POS), inc8(rd8(ABS(TRACK_READ_POS))));
            lda8(rd8(ILDPY(0x07)));
            if (fz) { ret(); return; }
            cmp8((u8)A, 0x09);
            if (fz) {
                lda8(0x01);
                wr8(ABS(TRACK_READ_POS), 0x01);
            }
            cmp8((u8)A, 0x80);
            if (fc) {
                and8(0x7F);
                wr8(ABS(0x027A), (u8)A);
                lda8(rd8(ILDPY(0x07)));
                and8(0x1F);
                wr8(ABS(0x027A), (u8)A);
                step = 1;
            } else {
                step = 3;
            }
        }
        if (step == 1) {
            wr8(ABS(0x027A), dec8(rd8(ABS(0x027A))));
            lda8(0x00);
            wr8(ABS(BUILD_COLUMN), 0);
            wr8(ABS(0x027E), 0);
        }
        if (step != 2) {
            /* L80ED8F */
            wr8(ABS(ROW_TYPE), (u8)A);
            cmp8((u8)A, 0x03);
            if (fz) wr8(DP(BONUS_SIGN), inc8(rd8(DP(BONUS_SIGN))));
        }
        /* L80ED98 */
        X = rd8(ABS(ROW_TYPE)); nz8((u8)X);
        push16(0xED6F); BonusRowList_m1x1();
        Y = rd8(ABS(0x027E)); nz8((u8)Y);
        lda8(rd8((((u32)DB << 16) + rd16(DP(0x05)) + Y) & 0xFFFFFF));
        if (!fz) break;
        wr8(ABS(0x027E), (u8)A);
        wr8(ABS(BUILD_COLUMN), (u8)A);
        step = 0;
    }
    push16(0xED7C); BlankQueueRowB_m1x1();
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x05)) + Y) & 0xFFFFFF));
    X = (u8)A; nz8((u8)X);
    push16(0xED82); CopyPieceColumn_m1x1();
    Y = inc8((u8)Y);
    Y = inc8((u8)Y);
    wr8(ABS(0x027E), (u8)Y);
    lda8(0x81); wr8(LONG(VQ0_VMAIN), 0x81);
    lda8(0x01); wr8(LONG(VQ0_DMAP), 0x01);
    lda8(0x18); wr8(LONG(VQ0_BBAD), 0x18);
    lda8(rd8(LONG(VQ_END)));
    fc = 0; adc8(0x0C);
    wr8(LONG(VQ0_SRC), (u8)A);
    lda8(0x00); wr8(LONG(VQ0_SRC_HI), 0);
    lda8(0x7F); wr8(LONG(VQ0_SRC_BANK), 0x7F);
    rep(0x20);
    lda16(rd16(LONG(VQ_END)));
    fc = 0; adc16(0x002A);
    wr16(LONG(VQ_END), A);
    wr16(LONG(VQ0_NEXT), A);
    lda16(rd16(ABS(TRACK_VRAM_COL)));
    fc = 0; adc16(0x07E0);
    wr16(LONG(VQ0_VMADD), A);
    lda16(0x001E); wr16(LONG(VQ0_SIZE), 0x001E);
    sep(0x20);
    lda8(rd8(ABS(TRACK_ROW_PTR)));
    wr8(DP(0x0F), (u8)A);
    push16(0xEDDE); CourseColumnB_m1x1();
    lda8(rd8(ABS(TRACK_ROW_PTR)));
    push16(0xEDE4); NextColumn_m1x1();
    wr8(ABS(TRACK_ROW_PTR), (u8)A);
    wr8(ABS(TRACK_VRAM_COL), inc8(rd8(ABS(TRACK_VRAM_COL))));
    lda8(rd8(ABS(TRACK_VRAM_COL)));
    cmp8((u8)A, 0x80);
    if (fz) {
        lda8(0x60); wr8(ABS(TRACK_VRAM_COL), 0x60);
        lda8(rd8(ABS(TRACK_VRAM_HALF)));
        eor8(0x04);
        wr8(ABS(TRACK_VRAM_HALF), (u8)A);
    }
    lda8(rd8(ABS(RIDER_COLUMN)));
    push16(0xEE04); NextColumn_m1x1();
    wr8(ABS(0x0279), (u8)A);
    ret();
}
/* $81:9E9F (RTS): draw object $03A3 as a sprite group: its type $02FF
 * picks a frame list ($81:ADB7), its frame $034F + 1 times 4 * the piece
 * count $0390 indexes it; for each piece (backwards, 4 bytes at
 * $81:ADCB: X, Y, tile, attributes) not flagged hidden ($20) an OAM
 * entry at $1600 + 32 * slot, offset by the object's X $027F / Y $028F,
 * attributes merged with its palette $038F, and the size bit ($10) in
 * $1820. */
void DrawObjectPieces_m1x1(void)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(ABSX(OBJ_TYPE)));
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    rep(0x20);
    lda16(rd16(ABSY(0xADB7)));
    wr16(DP(0x00), A);
    sep(0x20);
    lda8(rd8(ABSX(OBJ_FRAME)));
    lda8(inc8((u8)A));
    wr8(ABS(WRMPYA), (u8)A);
    lda8(rd8(ABSX(OBJ_SCRIPT + 0x01)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    wr8(ABS(WRMPYB), (u8)A);
    rep(0x30);
    lda16(rd16(ABS(RDMPYL)));
    fc = 1; sbc16(0x0004);
    fc = 0; adc16(rd16(DP(0x00)));
    Y = A; nz16(Y);
    lda16(rd16(ABSX(OBJ_SCRIPT + 0x01))); wr16(DP(0x00), A);
    lda16(rd16(ABSX(OBJ_X))); wr16(DP(0x04), A);
    lda16(rd16(ABSX(OBJ_SCRIPT))); wr16(DP(0x0E), A);
    lda16(rd16(ABSX(OBJ_Y))); wr16(DP(0x06), A);
    lda16(X);
    lda16(asl16(A)); lda16(asl16(A)); lda16(asl16(A)); lda16(asl16(A)); lda16(asl16(A));
    X = A; nz16(X);
    sep(0x20);
    do {
        WAIT_TICK();
        lda8(rd8(ABSY(0xADCE)));
        and8(0x20);
        if (fz) {
            lda8(rd8(ABSY(0xADCB)));
            fc = 0; adc8(rd8(DP(0x04)));
            wr8(ABSX(OAM_BUF), (u8)A);
            lda8(rd8(ABSY(0xADCC)));
            fc = 0; adc8(rd8(DP(0x06)));
            wr8(ABSX(OAM_BUF + 0x01), (u8)A);
            lda8(rd8(ABSY(0xADCD)));
            wr8(ABSX(OAM_BUF + 0x02), (u8)A);
            lda8(rd8(ABSY(0xADCE)));
            and8(0xCF);
            ora8(rd8(DP(0x0E)));
            wr8(ABSX(OAM_BUF + 0x03), (u8)A);
            rep(0x20);
            push16(X);
            lda16(X);
            lda16(lsr16(A)); lda16(lsr16(A));
            X = A; nz16(X);
            sep(0x20);
            lda8(rd8(ABSY(0xADCE)));
            and8(0x10);
            lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
            wr8(ABSX(OAM_SIZE_BUF), (u8)A);
            X = pull16(); nz16(X);
        }
        X = dec16(X); X = dec16(X); X = dec16(X); X = dec16(X);
        Y = dec16(Y); Y = dec16(Y); Y = dec16(Y); Y = dec16(Y);
        wr8(DP(0x00), dec8(rd8(DP(0x00))));
    } while (!fz);
    sep(0x10);
    ret();
}
/* $80:A9CA Mode03_Race (RTS): one Stadium 4 race frame: engine pitch;
 * leans and lane bands; the player pad (unless the attract demo $0CD7);
 * rider state steps, piece scripts and actions. After the finish ($84)
 * the finish step and clock; else overheating, a free run's rivals, rank
 * and ball, the race timer, item flash and rider collisions. Then crash
 * recovery, throttles, lanes, ground heights, shadows and markers, the
 * course column, ball or gauge, the riders sorted and drawn, their
 * shadows, the move step, start lights, countdown sign, turbo meter,
 * wheels, CPU riders, the place badge; at a lap ($AC) the course time
 * and stadium badge (and after the finish the label $80:AA89 at $1892);
 * pickups, markers, exhaust, confetti, rival engine, speed readout, best
 * time blink, bonus banner and jump banner. */
void Mode03_Race_m1x1(void)
{
    push16(0xA9CC); SendSpeedToApu_m1x1();
    lean_and_bands4(0xA9D1, 0xA9D4);
    lda8(rd8(ABS(ATTRACT_DEMO)));
    if (fz) { lda8(rd8(DP(P1_HELD_HI))); wr8(ABS(RIDER_INPUT), (u8)A); }
    push16(0xA9E4); RiderStepAll_m1x1();
    push16(0xA9E7); RunAllRiderPieces_m1x1();
    push16(0xA9EA); RunPieceActions_m1x1();
    lda8(rd8(DP(RACE_END)));
    if (!fz) {
        push16(0xA9F1); RaceFinishStep_m1x1();
        push16(0xA9F4); RaceClockStep_m1x1();
    } else {
        push16(0xA9FA); OverheatStep_m1x1();
        lda8(rd8(DP(FINAL_RACE)));
        if (!fz) {
            push16(0xAA01); SpawnRivals_m1x1();
            push16(0xAA04); RankPlayer_m1x1();
            push16(0xAA07); LaunchBall_m1x1();
        }
        push16(0xAA0A); RaceTimer_m1x1();
        push16(0xAA0D); ItemFlashState_m1x1();
        push16(0xAA10); RiderCollisions_m1x1();
    }
    push16(0xAA13); CrashRecovery_m1x1();
    push16(0xAA16); ThrottleAll_m1x1();
    push16(0xAA19); UpdateLanes_m1x1();
    push16(0xAA1C); RiderGroundHeights_m1x1();
    push16(0xAA1F); DrawShadowsAndMarkers_m1x1();
    push8(0x80); push16(0xAA23); LoadRaceColumn_m1x1();
    push16(0xAA26); BallOrGauge_m1x1();
    push16(0xAA29); SortAndDrawRiders_m1x1();
    push16(0xAA2C); PlaceAllRiderShadows_m1x1();
    push16(0xAA2F); RaceMoveStep_m1x1();
    push16(0xAA32); RaceStartLights_m1x1();
    push16(0xAA35); DrawCountdownSign_m1x1();
    push16(0xAA38); TurboMeter_m1x1();
    push16(0xAA3B); SpinAllWheels_m1x1();
    push16(0xAA3E); CpuRidersStep_m1x1();
    push16(0xAA41); QueuePlaceBadgeHud_m1x1();
    lda8(rd8(DP(0xAC)));
    if (!fz) {
        wr8(DP(0xAC), 0);
        push8(0x80); push16(0xAA4B); RecordCourseTime_m1x1();
        push8(0x80); push16(0xAA4F); CALL(QueueStadiumBadge_m1x1, 0x81CA37, 3);
        lda8(rd8(DP(RACE_END)));
        if (!fz) {
            rep(0x20);
            lda16(0x1892); wr16(DP(0x00), 0x1892);
            lda16(0xAA92); wr16(DP(0x02), 0xAA92);
            sep(0x20);
            lda8(0x24); wr8(DP(0x0F), 0x24);
            push16(0xAA68); VramQueueTiles_m1x1();
        }
    }
    push8(0x80); push16(0xAA6C); DrawCoinPickup_m1x1();
    push8(0x80); push16(0xAA70); DrawBonusPickup_m1x1();
    push8(0x80); push16(0xAA74); MarkerObjects_m1x1();
    push8(0x80); push16(0xAA78); SpawnExhaust_m1x1();
    push8(0x80); push16(0xAA7C); ConfettiAll_m1x1();
    push8(0x80); push16(0xAA80); RivalEngineSound_m1x1();
    push8(0x80); push16(0xAA84); SpeedReadout_m1x1();
    push8(0x80); push16(0xAA88); BestTimeBlink_m1x1();
    push8(0x80); push16(0xAA8C); StadiumBonusBanner_m1x1();
    push8(0x80); push16(0xAA90); StepJumpBanner_m1x1();
    ret();
}
/* $80:F29F Mode0C (RTS): one Stadium 4 attract-demo frame: engine pitch,
 * the CPU pads released, the demo input (carry set: only the end steps);
 * else the race frame rider steps without the pad, timers or rewards.
 * Then start lights, turbo meter, wheels, CPU riders, the pad display and
 * confetti. */
void Mode0C_m1x1(void)
{
    push16(0xF2A1); SendSpeedToApu_m1x1();
    ldx8(0x07);
    lda8(0xFF);
    do {
        wr8(DPX(RANDOM_LFSR), 0xFF);
        X = dec8((u8)X);
    } while (!fn);
    push16(0xF2AD); DemoInputStep_m1x1();
    if (!fc) {
        lean_and_bands4(0xF2B4, 0xF2B7);
        push16(0xF2BD); RiderStepAll_m1x1();
        push16(0xF2C0); RunAllRiderPieces_m1x1();
        push16(0xF2C3); RunPieceActions_m1x1();
        push16(0xF2C6); OverheatStep_m1x1();
        push16(0xF2C9); RiderCollisions_m1x1();
        push16(0xF2CC); CrashRecovery_m1x1();
        push16(0xF2CF); ThrottleAll_m1x1();
        push16(0xF2D2); UpdateLanes_m1x1();
        push16(0xF2D5); RiderGroundHeights_m1x1();
        push16(0xF2D8); DrawShadowsAndMarkers_m1x1();
        push8(0x80); push16(0xF2DC); LoadRaceColumn_m1x1();
        push16(0xF2DF); SortAndDrawRiders_m1x1();
        push16(0xF2E2); PlaceAllRiderShadows_m1x1();
        push16(0xF2E5); RaceMoveStep_m1x1();
    }
    push16(0xF2E8); RaceStartLights_m1x1();
    push16(0xF2EB); TurboMeterJmp_m1x1();
    push16(0xF2EE); SpinAllWheels_m1x1();
    push16(0xF2F1); CpuRidersStep_m1x1();
    push16(0xF2F4); DemoPadDisplay_m1x1();
    push8(0x80); push16(0xF2F8); ConfettiAll_m1x1();
    ret();
}
/* $80:B91B (RTS, M=0 X=0): A * $60 through Mul16b ($62 = A), the middle
 * word $65 back in A. Called from the IRQ with D = $1B00. */
void ScaleByScroll_m0x0(void)
{
    wr16(DP(0x62), A);
    sep(0x20);
    push16(0xB921); Mul16b_m1x0();
    rep(0x20);
    lda16(rd16(DP(0x65)));
    ret();
}
/* $80:B927 (RTS): the parallax line buffers: with the scroll $9C as the
 * multiplier, 256 words at $1000 from the scale table $1B68 indexed by
 * $82:9DB3 + $9E (factor $C0), then factors $60 / $80 / $B0 / $E0 into
 * $68-$6E and 256 words at $1200 from $82:9FB3 + $A2. Runs from the IRQ
 * with D = $1B00: the direct-page operands are $1B00 + n. */
void ParallaxLines_m1x1(void)
{
    rep(0x30);
    lda16(rd16(ABS(SCROLL_X)));
    wr16(DP(0x60), A);
    wr16(DP(0x6A), A);
    lda16(0x00C0);
    push16(0xB935); ScaleByScroll_m0x0();
    wr16(DP(0x68), A);
    X = rd16(ABS(BG1_VSCROLL)); nz16(X);
    Y = 0; nz16(Y);
    do {
        lda16(rd16(LONGX(0x829DB3)));
        push16(Y);
        and16(0x00FF);
        Y = A; nz16(Y);
        lda16(rd16(ABSY(0x1B68)));
        Y = pull16(); nz16(Y);
        wr16(ABSY(0x1000), A);
        X = inc16(X);
        Y = inc16(Y);
        Y = inc16(Y);
        cmp16(Y, 0x0200);
    } while (!fc);
    static const u8 dst[4] = { 0x68, 0x6A, 0x6C, 0x6E };
    static const u16 fac[4] = { 0x0060, 0x0080, 0x00B0, 0x00E0 };
    static const u16 back[4] = { 0xB95B, 0xB963, 0xB96B, 0xB973 };
    for (int i = 0; i < 4; i++) {
        lda16(fac[i]);
        push16(back[i]); ScaleByScroll_m0x0();
        wr16(DP(dst[i]), A);
    }
    X = rd16(ABS(ZOOM)); nz16(X);
    Y = 0; nz16(Y);
    do {
        lda16(rd16(LONGX(0x829FB3)));
        push16(Y);
        and16(0x00FF);
        Y = A; nz16(Y);
        lda16(rd16(ABSY(0x1B68)));
        Y = pull16(); nz16(Y);
        wr16(ABSY(0x1200), A);
        X = inc16(X);
        Y = inc16(Y);
        Y = inc16(Y);
        cmp16(Y, 0x0200);
    } while (!fc);
    sep(0x30);
    ret();
}
/* $80:924A (RTS): after a race the record $083A-$083C is kept for the
 * course ($083D + 3 * $7C) and $8A = $0708. In a race ($7D clear) the
 * HDMA channels come back, the course label ($80:92B8 + 5 * $7C) goes to
 * VRAM $1893, $88 = $40, $044F cleared, $A6 = $FC, $A4 = 4 and HDMA scene
 * 0. In a free run, sound $03 and the results screen: BG layout, $1E0C =
 * 1; the character name ($80:A598 + 15 * $154F, +$4B with $81:800C) at
 * $1844, player markers, the course number at $1854, places (badges at
 * $1858 / $1A08 when ranked, else the row $80:9462 at $1A04), best time
 * at $1915, item points, the time difference, the last distance, the
 * jump score ($156D) at $1AB6, the score digits $0CE5 at $1B16, and for
 * places 1-7 the place points ($80:944A / $9452, bonus $BA from $80:945A)
 * at $1A17. */
void RaceResultsSetup_m1x1(void)
{
    lda8(rd8(DP(COURSE_SLOT)));
    lda8(asl8((u8)A));
    fc = 0; adc8(rd8(DP(COURSE_SLOT)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABS(RECORD_TIME))); wr8(ABSX(0x083D), (u8)A);
    lda8(rd8(ABS(RECORD_TIME + 0x01))); wr8(ABSX(0x083E), (u8)A);
    lda8(rd8(ABS(RECORD_TIME + 0x02))); wr8(ABSX(0x083F), (u8)A);
    rep(0x20);
    lda16(0x0708); wr16(DP(TIMER_A), 0x0708);
    sep(0x20);
    lda8(rd8(DP(FINAL_RACE)));
    if (fz) {
        wr8(ABS(ANIM_UPLOAD_ON), 0);
        push8(0x80); push16(0x9276); SetupHdmaChannels_m1x1();
        lda8(rd8(DP(COURSE_SLOT)));
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        fc = 0; adc8(rd8(DP(COURSE_SLOT)));
        rep(0x20);
        and16(0x00FF);
        fc = 0; adc16(0x92B6);
        wr16(DP(0x02), A);
        lda16(0x1893); wr16(DP(0x00), 0x1893);
        sep(0x20);
        push16(0x929E); VramQueueTiles20_m1x1();
        lda8(0x40); wr8(ABS(MODE_TIMER), 0x40);
        wr8(ABS(ENTRY_CURSOR), 0);
        lda8(0xFC); wr8(DP(PIC_SCROLL), 0xFC);
        lda8(0x04); wr8(DP(0xA4), 0x04);
        lda8(0x00);
        push8(0x80); push16(0x92B4); LoadHdmaScene_m1x1();
        ret();
        return;
    }
    lda8(0x03);
    push8(0x80); push16(0x927F); QueueSoundB_m1x1();
    lda8(0x01);
    wr8(ABS(BG1SC), 0x01); wr8(ABS(0x1900), 0x01);
    lda8(0x09); wr8(ABS(BG2SC), 0x09);
    lda8(0x44); wr8(ABS(BG12NBA), 0x44);
    lda8(0x06); wr8(ABS(SH_TM), 0x06);
    wr8(ABS(SH_CGADSUB), 0);
    wr8(ABS(PALETTE_BUF), 0); wr8(ABS(PALETTE_BUF + 0x01), 0);
    wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
    lda8(0x01); wr8(ABS(ANIM_UPLOAD_ON), 0x01);
    lda8(rd8(ABS(PLAYER_CHAR)));
    wr8(ABS(WRMPYA), (u8)A);
    lda8(0x0F); wr8(ABS(WRMPYB), 0x0F);
    rep(0x20);
    A = rd16(ABS(RDMPYL));
    fc = 0; adc16(0xA5A1);
    wr16(DP(0x02), A);
    lda16(rd16(LONG(STADIUM_SELECT)));
    if (!fz) {
        lda16(rd16(DP(0x02)));
        fc = 0; adc16(0x004B);
        wr16(DP(0x02), A);
    }
    /* the original's name, or the roster's for an added character (bank $80) */
    wr16(DP(0x02), (u16)roster_name(rd8(LONG(STADIUM_SELECT)) != 0, rd8(ABS(PLAYER_CHAR)), rd16(DP(0x02))));
    lda16(0x1844); wr16(DP(0x00), 0x1844);
    sep(0x20);
    push16(0x9323); VramQueueTiles20_m1x1();
    push8(0x80); push16(0x9327); QueuePlayerMarkers_m1x1();
    lda8(rd8(DP(COURSE_SLOT)));
    lda8(asl8((u8)A));
    fc = 0; adc8(0x60);
    wr8(ABS(DIGIT_ROW), (u8)A);
    lda8(inc8((u8)A));
    wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
    fc = 0; adc8(0x0F);
    wr8(ABS(DIGIT_ROW + 0x02), (u8)A);
    lda8(inc8((u8)A));
    wr8(ABS(DIGIT_ROW + 0x03), (u8)A);
    lda8(0x22); wr8(ABS(DIGIT_BUF), 0x22);
    rep(0x20);
    lda16(0x1854); wr16(DP(0x00), 0x1854);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    push16(0x9354); VramQueueTiles20_m1x1();
    push16(0x9357); ComputePlaces_m1x1();
    lda8(rd8(ABS(RIDER_DIST)));
    ora8(rd8(ABS(RIDER_DIST + 0x01)));
    if (!fz) {
        rep(0x20);
        lda16(0x1858); wr16(DP(0x00), 0x1858);
        sep(0x20);
        lda8(0x20); wr8(DP(0x0F), 0x20);
        push16(0x936F); QueuePlaceBadge_m1x1();
    }
    rep(0x20);
    lda16(0x1915); wr16(DP(0x00), 0x1915);
    sep(0x20);
    push16(0x937B); QueueBestTime_m1x1();
    lda8(rd8(ABS(RIDER_DIST)));
    ora8(rd8(ABS(RIDER_DIST + 0x01)));
    if (!fz) {
        rep(0x20);
        lda16(0x1A08); wr16(DP(0x00), 0x1A08);
        sep(0x20);
        lda8(0x24); wr8(DP(0x0F), 0x24);
        push16(0x9393); QueuePlaceBadge_m1x1();
    } else {
        rep(0x20);
        lda16(0x1A04); wr16(DP(0x00), 0x1A04);
        lda16(0x9460); wr16(DP(0x02), 0x9460);
        sep(0x20);
        lda8(0x24); wr8(DP(0x0F), 0x24);
        push16(0x93AA); VramQueueTiles_m1x1();
    }
    push16(0x93AD); ShowItemPoints_m1x1();
    push16(0x93B0); ShowDigitDifference_m1x1();
    push16(0x93B3); ShowLastDistance_m1x1();
    rep(0x20);
    lda16(rd16(ABS(JUMP_BEST_SCORE)));
    push16(0x93BB); BuildScoreDigits_m0x1();
    rep(0x20);
    lda16(0x1AB6); wr16(DP(0x00), 0x1AB6);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x24); wr8(DP(0x0F), 0x24);
    push16(0x93D0); VramQueueTiles_m1x1();
    rep(0x20);
    lda16(0x0CE5); wr16(DP(0x02), 0x0CE5);
    lda16(0x1B16); wr16(DP(0x00), 0x1B16);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    ldy8(0x05);
    push8(0x80); push16(0x93E8); QueueDigitsAt_m1x1();
    X = rd8(ABS(PLAYER_PLACE)); nz8((u8)X);
    X = dec8((u8)X);
    cmp8((u8)X, 0x07);
    if (fc) { ret(); return; }
    lda8(rd8(ABSX(0x9448))); wr8(ABS(DIGIT_ROW), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x05), (u8)A);
    lda8(rd8(ABSX(0x9450))); wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x06), (u8)A);
    lda8(rd8(ABSX(0x9458))); wr8(DP(PLACE_BONUS), (u8)A);
    lda8(0x25); wr8(ABS(DIGIT_BUF), 0x25);
    wr8(ABS(DIGIT_ROW + 0x02), 0); wr8(ABS(DIGIT_ROW + 0x03), 0); wr8(ABS(DIGIT_ROW + 0x04), 0);
    lda8(0x10);
    wr8(ABS(DIGIT_ROW + 0x07), 0x10); wr8(ABS(DIGIT_ROW + 0x08), 0x10); wr8(ABS(DIGIT_ROW + 0x09), 0x10);
    lda8(rd8(ABSX(0x9448)));
    if (fz) {
        lda8(0xFF);
        wr8(ABS(DIGIT_ROW), 0xFF); wr8(ABS(DIGIT_ROW + 0x05), 0xFF);
    }
    rep(0x20);
    lda16(0x1A17); wr16(DP(0x00), 0x1A17);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x24); wr8(DP(0x0F), 0x24);
    push16(0x9446); VramQueueTiles_m1x1();
    ret();
}
/* $82:8180 ForceBlank (RTL): screen off ($0816 shadow), HDMA off. */
void ForceBlank_m1x1(void)
{
    lda8(0x80);
    wr8(ABS(INIDISP), 0x80);
    wr8(ABS(0x0816), 0x80);
    wr8(ABS(HDMAEN), 0);
    wr8(ABS(SH_HDMAEN), 0);
    retl();
}
/* $82:818F ClearTilemaps (RTL): VRAM $0000-$0FFF gets tile $FC with
 * attribute 0, $1800-$1BFF tile $FF; a fixed-source DMA of the byte at
 * $0000 into the low bytes, then $00 into the high bytes. */
void ClearTilemaps_m1x1(void)
{
    push16(D);
    lda8(0xFC);
    wr8(DP(0x00), 0xFC);
    rep(0x20);
    A = 0x4300; D = 0x4300;
    Y = 1;
    vram_fill_half4(0x0000, 0, 0x1000);
    wr16(DP(0x05), 0x1000);
    wr16(ABS(VMADDL), 0x0000);
    wr16(ABS(0x0000), 0);
    X = 0x80; wr8(ABS(VMAIN), 0x80);
    X = 0x19; wr8(DP(0x01), 0x19);
    wr16(DP(0x05), 0x1000);
    wr8(ABS(MDMAEN), 0x01);
    X = 0xFF; wr8(ABS(0x0000), 0xFF);
    vram_fill_half4(0x1800, 1, 0x0800);
    wr16(DP(0x05), 0x0800);
    wr16(ABS(VMADDL), 0x1800);
    wr16(ABS(0x0000), 0);
    X = 0x80; wr8(ABS(VMAIN), 0x80);
    X = 0x19; wr8(DP(0x01), 0x19);
    wr16(DP(0x05), 0x0800);
    wr8(ABS(MDMAEN), 0x01);
    A = 0x0800;
    sep(0x20);
    D = pull16(); nz16(D);
    retl();
}
/* $82:805F InitPpuRegisters (RTL), once from Reset: screen off; clear
 * $7E:2000-$7F:FFDF and $00:0000-$1E7F; the VRAM queue reset; the PPU
 * registers (D = $2100) and their shadows; the HDMA channels. */
void InitPpuRegisters_m1x1(void)
{
    push8(0x82); push16(0x8062); ForceBlank_m1x1();
    rep(0x20);
    wr16(DP(0x00), 0xE000); A = 0x2000; Y = 0x7E;
    push8(0x82); push16(0x8072); DmaFillZero_m0x1();
    wr16(DP(0x00), 0xFFE0); A = 0x0000; Y = 0x7F;
    push8(0x82); push16(0x8080); DmaFillZero_m0x1();
    wr16(DP(0x00), 0x1E80); A = 0x0000; Y = 0x00;
    push8(0x82); push16(0x808E); DmaFillZero_m0x1();
    rep(0x20);
    A = 0x0002;
    wr16(LONG(VQ_END), 0x0002);
    push16(D);
    A = 0x2100; D = 0x2100;
    sep(0x20);
    static const struct { u8 reg; u8 val; u16 shadow; } ppu[] = {
        {0x01, 0x01, 0},      {0x05, 0x09, 0x1904}, {0x06, 0x00, 0},
        {0x07, 0x01, 0x1900}, {0x08, 0x09, 0},      {0x0A, 0x00, 0},
        {0x09, 0x19, 0},      {0x0B, 0x44, 0},      {0x0C, 0x01, 0},
        {0x2C, 0x16, 0x1905}, {0x2D, 0x00, 0},      {0x2E, 0x00, 0},
        {0x2F, 0x00, 0},      {0x23, 0x00, 0},      {0x24, 0x00, 0},
        {0x25, 0x00, 0},      {0x30, 0x00, 0},      {0x31, 0x00, 0},
        {0x32, 0xE0, 0},      {0x33, 0x00, 0},
    };
    for (unsigned i = 0; i < sizeof ppu / sizeof ppu[0]; i++) {
        if (ppu[i].val) SET_A8(ppu[i].val);
        wr8(DP(ppu[i].reg), ppu[i].val);
        if (ppu[i].shadow) wr8(ABS(ppu[i].shadow), ppu[i].val);
    }
    static const u16 zero_shadows[] = {0x1907, 0x1908, 0x1909, 0x190A, 0x190B, 0x190C, 0x190D,
                                       0x190F, 0x1910, 0x1911};
    for (unsigned i = 0; i < sizeof zero_shadows / sizeof zero_shadows[0]; i++)
        wr8(ABS(zero_shadows[i]), 0);
    lda8(0x20); wr8(ABS(0x1912), 0x20);
    lda8(0x40); wr8(ABS(0x1913), 0x40);
    lda8(0x80); wr8(ABS(0x1914), 0x80);
    wr8(ABS(HDMAEN), 0);
    wr8(ABS(SH_HDMAEN), 0);
    push8(0x82); push16(0x8118); SetupHdmaChannels_m1x1();
    D = pull16(); nz16(D);
    sep(0x20);
    retl();
}
/* $80:824E DmaKick (M=0 X=1, RTS), with D = $4300: A = source address,
 * X = source bank, Y = channel mask for MDMAEN. */
void DmaKick_m0x1(void)
{
    wr16(DP(0x02), A);
    wr8(DP(0x04), (u8)X);
    wr8(ABS(MDMAEN), (u8)Y);
    ret();
}
/* $80:88BD (RTS): start a random course: NMI off and forced blank; the
 * course index is how many thresholds ($80:88B7) the random byte $7F:FFFF
 * passes ($7C = index / 2, $7D = its low bit); $0100 bytes of tiles
 * ($80:89AA by 3 * $7D) to VRAM $00C0; NMI back, $75 = 1, $1923 + 1; the
 * course number label (tile $10 + 2 * $7C, palette $42) at $80:89B0 by
 * $7D; the PPU registers for the race screen and the HDMA channels. */
void StartRandomCourse_m1x1(void)
{
    wr8(ABS(SH_CGADSUB), 0);
    wr8(ABS(NMITIMEN), 0);
    push8(0x80); push16(0x88C6); ForceBlank_m1x1();
    ldx8(0xFF); wr8(DP(0x00), 0xFF);
    lda8(rd8(LONG(BCAST_MINUTE)));
    do {
        WAIT_TICK();
        wr8(DP(0x00), inc8(rd8(DP(0x00))));
        X = inc8((u8)X);
        cmp8((u8)A, rd8(ABSX(0x88B5)));
    } while (fc);
    lda8(rd8(DP(0x00)));
    lda8(lsr8((u8)A));
    wr8(DP(0x7C), (u8)A);
    lda8(rol8((u8)A));
    and8(0x01);
    wr8(DP(0x7D), (u8)A);
    lda8(asl8((u8)A));
    fc = 0; adc8(rd8(DP(0x7D)));
    wr8(DP(0x00), (u8)A);
    rep(0x20);
    push16(D);
    lda16(0x4300);
    D = A;
    ldy8(0x80); wr8(ABS(VMAIN), 0x80);
    lda16(0x1801); wr16(DP(0x00), 0x1801);
    lda16(0x00C0); wr16(ABS(VMADDL), 0x00C0);
    lda16(0x0100); wr16(DP(0x05), 0x0100);
    Y = rd8(ABS(0x0000)); nz8((u8)Y);
    lda16(rd16(ABSY(0x89A8)));
    X = rd8(ABSY(0x89AA)); nz8((u8)X);
    ldy8(0x01);
    push16(0x8910); DmaKick_m0x1();
    D = pull16(); nz16(D);
    sep(0x20);
    lda8(rd8(ABS(SH_NMITIMEN))); wr8(ABS(NMITIMEN), (u8)A);
    lda8(0x01); wr8(DP(0x75), 0x01);
    wr8(ABS(FADE_STATE), inc8(rd8(ABS(FADE_STATE))));
    lda8(0x42); wr8(ABS(DIGIT_BUF), 0x42);
    lda8(rd8(DP(0x7C)));
    lda8(asl8((u8)A));
    ora8(0x10);
    for (int k = 0; k < 4; k++) {
        if (k) { fc = 0; adc8(0x0F); }
        wr8(ABS(DIGIT_ROW + 2 * k), (u8)A);
        lda8(inc8((u8)A));
        wr8(ABS(DIGIT_ROW + 0x01 + 2 * k), (u8)A);
    }
    lda8(rd8(DP(0x7D)));
    and8(0x01);
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    rep(0x20);
    lda16(rd16(ABSY(0x89AE))); wr16(DP(0x00), A);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x18); wr8(DP(0x0F), 0x18);
    push16(0x896A); VramQueueTiles_m1x1();
    lda8(0x01); wr8(ABS(OBSEL), 0x01);
    lda8(0x09); wr8(ABS(BGMODE), 0x09); wr8(ABS(0x1904), 0x09);
    wr8(ABS(MOSAIC), 0);
    lda8(0x01); wr8(ABS(BG1SC), 0x01); wr8(ABS(0x1900), 0x01);
    lda8(0x09); wr8(ABS(BG2SC), 0x09);
    wr8(ABS(BG4SC), 0);
    lda8(0x19); wr8(ABS(BG3SC), 0x19);
    lda8(0x44); wr8(ABS(BG12NBA), 0x44);
    lda8(0x01); wr8(ABS(BG34NBA), 0x01);
    lda8(0x17); wr8(ABS(SH_TM), 0x17);
    wr8(DP(0x86), 0);
    wr8(DP(0x87), 0);
    push8(0x80); push16(0x89A6); SetupHdmaChannels_m1x1();
    ret();
}
/* $80:B2D6 (RTS): screen off ($1903 shadow). */
void ScreenOff_m1x1(void)
{
    lda8(0x80);
    wr8(ABS(INIDISP), 0x80);
    wr8(ABS(SH_INIDISP), 0x80);
    ret();
}
/* $80:85A9 RunGameModeScript (RTS): with $75 set, run mode $76's frame
 * routine ($80:B4F9). Otherwise (unless $50) run the next step $77 of mode
 * $76's setup script ($80:B52B); before the first step, screen off, the
 * BS-X boot check ($82:9182) and $8F-$0815 cleared. */
void RunGameModeScript_m1x1(void)
{
    lda8(rd8(DP(MODE_ACTIVE)));
    if (!fz) {
        lda8(rd8(DP(GAME_MODE)));
        lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSY(0xB502))); wr8(DP(0x00), (u8)A);
        Y = inc8((u8)Y);
        lda8(rd8(ABSY(0xB502))); wr8(DP(0x01), (u8)A);
        if (!r816_call_pc(0x800000 | rd16(0x0000)))
            r816_unknown_target(0x8085BC, rd16(0x0000));
        return;
    }
    lda8(rd8(DP(MODE_HOLD)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(GAME_MODE)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0xB534))); wr8(DP(0x02), (u8)A);
    X = inc8((u8)X);
    lda8(rd8(ABSX(0xB534))); wr8(DP(0x03), (u8)A);
    Y = rd8(DP(MODE_STEP)); nz8((u8)Y);
    wr8(DP(MODE_STEP), inc8(rd8(DP(MODE_STEP))));
    Y = dec8((u8)Y);
    if (!fn) {
        lda8((u8)Y);
        lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(IDPY(0x02))); wr8(DP(0x00), (u8)A);
        Y = inc8((u8)Y);
        lda8(rd8(IDPY(0x02))); wr8(DP(0x01), (u8)A);
        if (!r816_call_pc(0x800000 | rd16(0x0000)))
            r816_unknown_target(0x8085E7, rd16(0x0000));
        return;
    }
    push16(0x85EC); ScreenOff_m1x1();
    push8(0x80); push16(0x85F0); CALL(LoadScene_m1x1, 0x829182, 3);
    rep(0x10);
    lda8(0x00);
    X = 0x0815; nz16(X);
    do {
        wr8(DPX(0x00), (u8)A);
        X = dec16(X);
        cmp16(X, 0x008F);
    } while (!fz);
    sep(0x10);
    ret();
}
/* $80:83D2 NmiVramUpload (RTS), in vblank with D = $4300: the four
 * animated tile pairs ($1E12-$1E18 from banks $1550-$1553, each $0100
 * bytes to VRAM $2000/$2100, $2080/$2180, $2200/$2300, $2280/$2380, the
 * second half from +$200) when $1E12 is set; $0200 bytes from $9B:($86)
 * to $5A00; the palette ($1C00, $0200 bytes) when $1915 is set; $0200
 * bytes from $9B:($1E0A) to ($1E10) when $1E0C and $1E10 are set; then
 * OAM ($1600, $0220 bytes). */
void NmiVramUpload_m1x1(void)
{
    push16(D);
    A = 0x4300; nz8(0);
    D = A; nz16(D);
    rep(0x10);
    lda8(0x80); wr8(ABS(VMAIN), 0x80);
    Y = 0x1801; nz16(Y); wr16(DP(0x00), 0x1801);
    rep(0x20);
    sep(0x10);
    lda16(rd16(ABS(0x1E12)));
    if (!fz) {
        static const u16 src[4] = { 0x1E12, 0x1E14, 0x1E16, 0x1E18 };
        static const u16 vram[4] = { 0x2000, 0x2080, 0x2200, 0x2280 };
        static const u16 back[4][2] = {
            { 0x8406, 0x841B }, { 0x8431, 0x8446 }, { 0x845C, 0x8471 }, { 0x8487, 0x849C },
        };
        for (int k = 0; k < 4; k++) {
            lda16(vram[k]); wr16(ABS(VMADDL), vram[k]);
            lda16(0x0100); wr16(DP(0x05), 0x0100);
            lda16(rd16(ABS(src[k])));
            X = rd8(ABS(0x1550 + k)); nz8((u8)X);
            ldy8(0x01);
            push16(back[k][0]); DmaKick_m0x1();
            u16 v2 = (u16)(vram[k] + 0x0100);
            lda16(v2); wr16(ABS(VMADDL), v2);
            lda16(0x0100); wr16(DP(0x05), 0x0100);
            lda16(rd16(ABS(src[k])));
            fc = 0; adc16(0x0200);
            push16(back[k][1]); DmaKick_m0x1();
        }
    }
    rep(0x20);
    sep(0x10);
    lda16(rd16(ABS(UPLOAD_SRC)));
    if (!fz) {
        lda16(0x5A00); wr16(ABS(VMADDL), 0x5A00);
        lda16(0x0200); wr16(DP(0x05), 0x0200);
        lda16(rd16(ABS(UPLOAD_SRC)));
        ldx8(0x9B);
        ldy8(0x01);
        push16(0x84BA); DmaKick_m0x1();
    }
    rep(0x10);
    sep(0x20);
    lda8(rd8(ABS(PALETTE_DIRTY)));
    if (!fz) {
        wr8(ABS(CGADD), 0);
        Y = 0x2200; nz16(Y); wr16(DP(0x00), 0x2200);
        Y = 0x1C00; nz16(Y); wr16(DP(0x02), 0x1C00);
        lda8(0x00); wr8(DP(0x04), 0);
        Y = 0x0200; nz16(Y); wr16(DP(0x05), 0x0200);
        lda8(0x01); wr8(ABS(MDMAEN), 0x01);
        wr8(ABS(PALETTE_DIRTY), 0);
    }
    lda8(rd8(ABS(ANIM_UPLOAD_ON)));
    if (!fz) {
        rep(0x20);
        sep(0x10);
        lda16(rd16(ABS(0x1E10)));
        if (!fz) {
            ldy8(0x80); wr8(ABS(VMAIN), 0x80);
            lda16(0x1801); wr16(DP(0x00), 0x1801);
            lda16(rd16(ABS(0x1E10))); wr16(ABS(VMADDL), A);
            lda16(0x0200); wr16(DP(0x05), 0x0200);
            lda16(rd16(ABS(0x1E0A)));
            ldx8(0x9B);
            ldy8(0x01);
            push16(0x850E); DmaKick_m0x1();
        }
    }
    rep(0x20);
    sep(0x10);
    wr16(ABS(OAMADDL), 0);
    lda16(0x0400); wr16(DP(0x00), 0x0400);
    lda16(0x1600); wr16(DP(0x02), 0x1600);
    wr16(DP(0x04), 0);
    lda16(0x0220); wr16(DP(0x05), 0x0220);
    ldy8(0x01); wr8(ABS(MDMAEN), 0x01);
    lda16(0x1801); wr16(DP(0x00), 0x1801);
    sep(0x30);
    D = pull16(); nz16(D);
    ret();
}
/* $80:9CDF EndScreenStart (RTS): on a new Start press, with $81:800C set
 * go to mode $12 ($75 / $77 cleared, $1923 - 1); clear, leave for the
 * BIOS. */
void EndScreenStart_m1x1(void)
{
    if (port_end_screen_start())        /* the port restarts the hour (roster.h) */
        return;
    lda8(rd8(DP(P1_PRESS_HI)));
    and8(0x10);
    if (fz) { ret(); return; }
    lda8(rd8(LONG(STADIUM_SELECT)));
    if (fz) { bsx_exit4(0x809CFE); return; }
    lda8(0x12); wr8(DP(GAME_MODE), 0x12);
    wr8(DP(MODE_ACTIVE), 0);
    wr8(DP(MODE_STEP), 0);
    wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
    ret();
}
/* $80:9EA8 Mode12_AfterEnd (RTS): the closing credits: the BS-X end check;
 * while $88 counts down the fade ($1903 = 15 - ($88 - 1) / 16, colour math
 * $1C00 = 9); every fourth frame the bar and colour cycle and the scroll
 * $A6 + 1 (at $0700: HDMA off, $88 = $FF, $1905 = 5, $1903 = 0). Below
 * $0780 the next credits rows ($7E:4B00 + 8 * ($A6 & ~7), $0220 bytes) go to
 * VRAM $3000 + 4 * (($A6 + $E0) & $1F8); at $0780 the scroll stops, and
 * after $01C0 frames ($8A) Start leaves for the BIOS. */
void Mode12_AfterEnd_m1x1(void)
{
    push16(0x9EAA); BsxEndIfFlagged_m1x1();
    lda8(rd8(DP(MODE_TIMER)));
    if (!fz) {
        wr8(DP(MODE_TIMER), dec8(rd8(DP(MODE_TIMER))));
        lda8(dec8((u8)A));
        lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
        wr8(DP(0x00), (u8)A);
        lda8(0x0F);
        fc = 1; sbc8(rd8(DP(0x00)));
        wr8(ABS(SH_INIDISP), (u8)A);
        lda8(0x09); wr8(ABS(PALETTE_BUF), 0x09);
        wr8(ABS(PALETTE_BUF + 0x01), 0);
        wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
        lda8(0xFF); wr8(ABS(COLOUR_CYCLE), 0xFF);
    }
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x03);
    if (fz) {
        push16(0x9ED8); GrowBar087E_m1x1();
        push16(0x9EDB); StepColourCycle_m1x1();
        rep(0x20);
        wr16(DP(PIC_SCROLL), inc16(rd16(DP(PIC_SCROLL))));
        lda16(rd16(DP(PIC_SCROLL)));
        cmp16(A, 0x0700);
        if (fz) {
            sep(0x20);
            wr8(ABS(SH_HDMAEN), 0);
            lda8(0xFF); wr8(DP(MODE_TIMER), 0xFF);
            lda8(0x05); wr8(ABS(SH_TM), 0x05);
            wr8(ABS(SH_INIDISP), 0);
        }
    }
    rep(0x20);
    lda16(rd16(DP(0xC1)));
    if (fz) {
        lda16(rd16(DP(PIC_SCROLL)));
        cmp16(A, 0x0780);
        if (fn || !fc) {
            lda16(rd16(DP(PIC_SCROLL)));
            and16(0xFFF8);
            lda16(asl16(A)); lda16(asl16(A)); lda16(asl16(A));
            fc = 0; adc16(0x4B00);
            wr16(DP(0x02), A);
            sep(0x20);
            lda8(0x7E); wr8(DP(0x04), 0x7E);
            rep(0x20);
            lda16(0x0220); wr16(DP(0x10), 0x0220);
            lda16(rd16(ABS(PIC_SCROLL)));
            fc = 0; adc16(0x00E0);
            and16(0x01F8);
            lda16(asl16(A)); lda16(asl16(A));
            fc = 0; adc16(0x3000);
            wr16(DP(0x00), A);
            sep(0x20);
            push16(0x9F7D); QueueTileRows_m1x1();
            ret();
            return;
        }
        wr16(DP(0xC1), inc16(rd16(DP(0xC1))));
        lda16(0x01C0); wr16(DP(TIMER_A), 0x01C0);
    }
    lda16(0x0780); wr16(DP(PIC_SCROLL), 0x0780);
    lda16(rd16(DP(TIMER_A)));
    if (!fz) {
        wr16(DP(TIMER_A), dec16(rd16(DP(TIMER_A))));
    } else {
        lda16(rd16(DP(P1_PRESS_HI)));
        and16(0x0010);
        if (!fz) {
            sep(0x20);
            bsx_exit4(0x809F3A);
            return;
        }
    }
    sep(0x20);
    ret();
}
/* $82:8F27 (RTL): the save checksum: $10:5004 = the 16-bit sum of the
 * bytes $10:5000-$5FFF, $11:5000-$5FFF and $12:5000-$5FFF, $10:5002 its
 * complement (both cleared to 0 / $FFFF first). */
void SaveChecksum_m1x1(void)
{
    rep(0x30);
    lda16(0x0000);
    wr16(LONG(SAVE_SUM), 0);
    A = dec16(A);
    wr16(LONG(SAVE_SUM_INV), A);
    wr16(DP(0x00), 0);
    X = 0x0FFF; nz16(X);
    do {
        WAIT_TICK();
        lda16(rd16(LONGX(0x105000))); and16(0x00FF); wr16(DP(0x02), A);
        lda16(rd16(LONGX(0x115000))); and16(0x00FF); wr16(DP(0x04), A);
        lda16(rd16(LONGX(0x125000))); and16(0x00FF);
        fc = 0; adc16(rd16(DP(0x00)));
        fc = 0; adc16(rd16(DP(0x02)));
        fc = 0; adc16(rd16(DP(0x04)));
        wr16(DP(0x00), A);
        X = dec16(X);
    } while (!fn);
    wr16(LONG(SAVE_SUM), A);
    eor16(0xFFFF);
    wr16(LONG(SAVE_SUM_INV), A);
    sep(0x30);
    retl();
}
/* $82:8E8E (RTL): check the save header at $11:5700 against the ROM
 * title ($00:FFC0-$FFCF), the stadium flag $81:800C and the broadcast id
 * $7F:FFF0; on a mismatch clear $11:5700-$57FF, copy the title, and set
 * the starting values ($11:5720/23/26/29 = 4, $11:572C = $10). Then record
 * the broadcast id, date ($7F:FFF2, $7F:FFF9), the stadium flag and this
 * weekday's entry ($80:9020 (bank DB) by $7F:FFF3 & 7) and redo the checksum. */
void SaveHeaderCheck_m1x1(void)
{
    int same = 1;
    ldx8(0x0F);
    do {
        lda8(rd8(LONGX(ROM_TITLE)));
        cmp8((u8)A, rd8(LONGX(SAVE_TITLE)));
        if (!fz) { same = 0; break; }
        X = dec8((u8)X);
    } while (!fn);
    if (same) {
        lda8(rd8(LONG(SAVE_STADIUM)));
        cmp8((u8)A, rd8(LONG(STADIUM_SELECT)));
        if (!fz) same = 0;
    }
    if (same) {
        rep(0x20);
        lda16(rd16(LONG(BSX_CLOCK_0)));
        cmp16(A, rd16(LONG(SAVE_CLOCK)));
        if (!fz) same = 0;
    }
    if (!same) {
        rep(0x30);
        X = 0x00FE; nz16(X);
        lda16(0x0000);
        do {
            wr16(LONGX(SAVE_TITLE), 0);
            X = dec16(X); X = dec16(X);
        } while (!fn);
        sep(0x10);
        ldx8(0x0E);
        do {
            lda16(rd16(LONGX(ROM_TITLE)));
            wr16(LONGX(SAVE_TITLE), A);
            X = dec8((u8)X); X = dec8((u8)X);
        } while (!fn);
        lda16(0x0004);
        wr16(LONG(SAVE_TIMES), 4); wr16(LONG(SAVE_TIMES + 0x03), 4);
        wr16(LONG(SAVE_TIMES + 0x06), 4); wr16(LONG(SAVE_TIMES + 0x09), 4);
        lda16(0x0010); wr16(LONG(SAVE_BEST_TIME), 0x0010);
    }
    lda16(rd16(LONG(BSX_CLOCK_0))); wr16(LONG(SAVE_CLOCK), A);
    lda16(rd16(LONG(BSX_CLOCK_2))); wr16(LONG(SAVE_CLOCK_HI), A);
    lda16(rd16(LONG(BCAST_HOUR))); and16(0x00FF); wr16(LONG(SAVE_HOUR), A);
    lda16(rd16(LONG(STADIUM_SELECT))); wr16(LONG(SAVE_STADIUM), A);
    sep(0x20);
    lda8(rd8(LONG(BSX_CLOCK_3)));
    and8(0x07);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0x9020))); wr8(LONGX(SAVE_DAYS), (u8)A);
    push8(0x82); push16(0x8F23); SaveChecksum_m1x1();
    sep(0x30);
    retl();
}
/* $82:8F6F (RTL, DB = $82): keep the broadcast's records in the save: a total time
 * $084D-$084F below the saved best ($11:572C) replaces it with the course
 * times $083D-$0848 ($11:5720) and sets $085C; a score $0CE5-$0CE9 above
 * the saved one ($11:5730) replaces it and sets $085D. Then the broadcast
 * id, date, stadium flag and weekday entry, and the checksum. */
void SaveBestRecords_m1x1(void)
{
    push8(DB);
    lda8(0x82);
    push8(0x82); DB = pull8(); nz8(DB);
    wr8(ABS(0x085C), 0);
    wr8(ABS(0x085D), 0);
    int better = 0;
    lda8(rd8(ABS(TOTAL_TIME)));
    cmp8((u8)A, rd8(LONG(SAVE_BEST_TIME)));
    if (!fz) better = !fc;
    else {
        lda8(rd8(ABS(TOTAL_TIME + 0x01)));
        cmp8((u8)A, rd8(LONG(SAVE_BEST_SEC)));
        if (!fz) better = !fc;
        else {
            lda8(rd8(ABS(TOTAL_TIME + 0x02)));
            cmp8((u8)A, rd8(LONG(SAVE_BEST_FRAC)));
            better = !fc;
        }
    }
    if (better) {
        lda8(rd8(ABS(TOTAL_TIME))); wr8(LONG(SAVE_BEST_TIME), (u8)A);
        lda8(rd8(ABS(TOTAL_TIME + 0x01))); wr8(LONG(SAVE_BEST_SEC), (u8)A);
        lda8(rd8(ABS(TOTAL_TIME + 0x02))); wr8(LONG(SAVE_BEST_FRAC), (u8)A);
        wr8(ABS(0x085C), inc8(rd8(ABS(0x085C))));
        ldx8(0x0B);
        do {
            lda8(rd8(ABSX(0x083D))); wr8(LONGX(SAVE_TIMES), (u8)A);
            X = dec8((u8)X);
        } while (!fn);
    }
    ldx8(0x00);
    for (;;) {
        lda8(rd8(LONGX(SAVE_BEST_SCORE)));
        cmp8((u8)A, rd8(ABSX(SCORE_DIGITS)));
        if (!fc) {
            ldx8(0x04);
            do {
                lda8(rd8(ABSX(SCORE_DIGITS))); wr8(LONGX(SAVE_BEST_SCORE), (u8)A);
                X = dec8((u8)X);
            } while (!fn);
            wr8(ABS(0x085D), inc8(rd8(ABS(0x085D))));
            break;
        }
        if (!fz) break;
        X = inc8((u8)X);
        cmp8((u8)X, 0x05);
        if (fc) break;
    }
    lda8(rd8(LONG(BSX_CLOCK_0))); wr8(LONG(SAVE_CLOCK), (u8)A);
    lda8(rd8(LONG(BSX_CLOCK_1))); wr8(LONG(0x115711), (u8)A);
    lda8(rd8(LONG(BSX_CLOCK_2))); wr8(LONG(SAVE_CLOCK_HI), (u8)A);
    lda8(rd8(LONG(BCAST_HOUR))); wr8(LONG(SAVE_STADIUM), (u8)A);
    lda8(rd8(LONG(STADIUM_SELECT))); wr8(LONG(SAVE_STADIUM), (u8)A);
    lda8(rd8(LONG(BSX_CLOCK_3)));
    and8(0x07);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0x9020))); wr8(LONGX(SAVE_DAYS), (u8)A);
    push8(0x82); push16(0x901D); SaveChecksum_m1x1();
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:97B7 (RTS): the broadcast totals screen: BG3 / tile bases, the
 * saved best total ($11:572C) at $09D3 and best score ($11:5730) at $0A15;
 * the four course times $083D-$0848 rounded through $80:C470 and summed
 * (minutes, seconds, hundredths) into $084D-$084F and $0849-$084B; the
 * records saved; $1905 = $17, $1923 + 1, $75 + 1, $044F/$0450 cleared, $88
 * = $20, $A2 = $D8, $190D cleared. */
void BroadcastTotals_m1x1(void)
{
    lda8(0x19); wr8(ABS(BG3SC), 0x19);
    lda8(0x44); wr8(ABS(BG12NBA), 0x44);
    lda8(0x01); wr8(ABS(BG34NBA), 0x01);
    rep(0x20);
    lda16(0x09D3); wr16(DP(0x00), 0x09D3);
    lda16(0x572C); wr16(DP(0x18), 0x572C);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x97DB); QueueTimeDigits_m1x1();
    rep(0x20);
    lda16(0x0A15); wr16(DP(0x00), 0x0A15);
    lda16(0x5730); wr16(DP(0x18), 0x5730);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x97F1); QueueFiveDigits_m1x1();
    wr8(ABS(TOTAL_TIME), 0);
    wr8(ABS(TOTAL_TIME + 0x01), 0);
    wr8(ABS(TOTAL_TIME + 0x02), 0);
    ldx8(0x00);
    do {
        Y = rd8(ABSX(0x083F)); nz8((u8)Y);
        lda8(rd8(ABSY(0xC479))); wr8(ABSX(0x083F), (u8)A);
        fc = 0; adc8(rd8(ABS(TOTAL_TIME + 0x02)));
        wr8(ABS(TOTAL_TIME + 0x02), (u8)A);
        cmp8((u8)A, 0x64);
        if (fc) {
            sbc8(0x64);
            wr8(ABS(TOTAL_TIME + 0x02), (u8)A);
            u8 v = inc8(rd8(ABS(TOTAL_TIME + 0x01))); wr8(ABS(TOTAL_TIME + 0x01), v);
            if (fz) wr8(ABS(TOTAL_TIME), inc8(rd8(ABS(TOTAL_TIME))));
        }
        lda8(rd8(ABSX(0x083E)));
        fc = 0; adc8(rd8(ABS(TOTAL_TIME + 0x01)));
        wr8(ABS(TOTAL_TIME + 0x01), (u8)A);
        cmp8((u8)A, 0x3C);
        if (fc) {
            sbc8(0x3C);
            wr8(ABS(TOTAL_TIME + 0x01), (u8)A);
            wr8(ABS(TOTAL_TIME), inc8(rd8(ABS(TOTAL_TIME))));
        }
        lda8(rd8(ABSX(0x083D)));
        fc = 0; adc8(rd8(ABS(TOTAL_TIME)));
        wr8(ABS(TOTAL_TIME), (u8)A);
        lda8(rd8(ABS(TOTAL_TIME))); wr8(ABS(0x0849), (u8)A);
        lda8(rd8(ABS(TOTAL_TIME + 0x01))); wr8(ABS(0x084A), (u8)A);
        lda8(rd8(ABS(TOTAL_TIME + 0x02))); wr8(ABS(0x084B), (u8)A);
        X = inc8((u8)X); X = inc8((u8)X); X = inc8((u8)X);
        cmp8((u8)X, 0x0C);
    } while (!fc);
    push8(0x80); push16(0x985A); SaveBestRecords_m1x1();
    lda8(0x17); wr8(ABS(SH_TM), 0x17);
    wr8(ABS(FADE_STATE), inc8(rd8(ABS(FADE_STATE))));
    wr8(DP(MODE_ACTIVE), inc8(rd8(DP(MODE_ACTIVE))));
    wr8(ABS(END_STEP), 0);
    wr8(ABS(ENTRY_CURSOR), 0);
    lda8(0x20); wr8(DP(MODE_TIMER), 0x20);
    lda8(0xD8); wr8(DP(ZOOM), 0xD8);
    wr8(ABS(SH_CGADSUB), 0);
    ret();
}
/* $80:8F67 (RTS): new session: the save header check, the attract state
 * $0CD3-$0CF2 and $7A-$85 cleared, $79 = $7E = 1. */
void StartSession_m1x1(void)
{
    push8(0x80); push16(0x8F6A); SaveHeaderCheck_m1x1();
    ldy8(0x1F);
    lda8(0x00);
    do {
        wr8(ABSY(0x0CD3), 0);
        Y = dec8((u8)Y);
    } while (!fn);
    ldy8(0x0B);
    do {
        wr8(ABSY(0x007A), 0);
        Y = dec8((u8)Y);
    } while (!fn);
    wr8(DP(COURSE_SLOT), 0);
    wr8(ABS(0x007B), 0);
    lda8(0x01);
    wr8(DP(0x79), 0x01);
    wr8(DP(0x7E), 0x01);
    lda8(0x80);
    lda8(0x0F);
    ret();
}
/* $82:846B (RTL): the three entry-code digits $1570-$1572 mapped through
 * $82:84BA, their lower tiles + $20 in $1573-$1575, and the row ($156F,
 * palette $10) queued at VRAM $08F9. */
void QueueEntryDigits_m1x1(void)
{
    rep(0x20);
    lda16(0x08F9); wr16(DP(0x00), 0x08F9);
    lda16(0x156F); wr16(DP(0x02), 0x156F);
    sep(0x20);
    static const u16 back[3] = { 0x847F, 0x848F, 0x849F };
    for (int k = 0; k < 3; k++) {
        lda8(rd8(ABS(ENTRY_DIGITS + k)));
        push8(0x82); push16(back[k]); ByteTable84BA_m1x1();
        wr8(ABS(ENTRY_DIGITS + k), (u8)A);
        fc = 0; adc8(0x20);
        wr8(ABS(0x1573 + k), (u8)A);
    }
    lda8(0x10); wr8(DP(0x0F), 0x10);
    push8(0x82); push16(0x84B0); VramQueueTilesLong_m1x1();
    retl();
}
/* $81:CA37 QueueStadiumBadge (RTL): the badge for stadium $0928 (tiles
 * from $81:CA75, palette $22 in $0CF8-$0CFC) at VRAM $1894; for stadium 5
 * in a free run ($7D) the $0204 rows at $81:CA81 to $1892 instead. */
void QueueStadiumBadge_m1x1(void)
{
    X = rd8(ABS(LAP)); nz8((u8)X);
    cmp8((u8)X, 0x05);
    if (fz) {
        lda8(rd8(DP(FINAL_RACE)));
        if (!fz) {
            rep(0x20);
            lda16(0x1892); wr16(DP(0x00), 0x1892);
            lda16(0xCA81); wr16(DP(0x02), 0xCA81);
            lda16(0x0204); wr16(DP(0x10), 0x0204);
            sep(0x20);
            lda8(0x81); wr8(DP(0x04), 0x81);
            QueueTileRowsLong_m1x1();
            return;
        }
    }
    X = dec8((u8)X);
    lda8(0x94); wr8(DP(0x00), 0x94);
    lda8(0x18); wr8(DP(0x01), 0x18);
    lda8(rd8(LONGX(0x81CA75))); wr8(ABS(0x0CF9), (u8)A);
    lda8(inc8((u8)A)); wr8(ABS(0x0CFA), (u8)A);
    fc = 0; adc8(0x0F); wr8(ABS(0x0CFB), (u8)A);
    lda8(inc8((u8)A)); wr8(ABS(0x0CFC), (u8)A);
    lda8(0x22); wr8(ABS(0x0CF8), 0x22);
    lda8(0xF8); wr8(DP(0x02), 0xF8);
    lda8(0x0C); wr8(DP(0x03), 0x0C);
    lda8(0x24); wr8(DP(0x0F), 0x24);
    VramQueueTilesLong_m1x1();
}
/* $81:C8E1 QueueHalfUnits (RTL by tail jump, M=0 X=1, A = count of half
 * units): A / 2 as hundreds, tens and ones (tiles $03D0 / $03D1 / $03D3,
 * lower halves + $10), the half as ".0" / ".5" ($03D4), spacer and unit
 * tiles ($FF, $A6, $A4), header $27 at $03CF; queued through
 * VramQueueTilesLong (the caller set $00 / $02 / $0F). */
void QueueHalfUnits_m0x1(void)
{
    wr16(DP(0x04), A);
    lda16(lsr16(A));
    ldy8(0x00);
    for (;;) {
        cmp16(A, 0x0064);
        if (!fc) break;
        sbc16(0x0064);
        Y = inc8((u8)Y);
    }
    sep(0x20);
    push8((u8)A);
    wr8(ABS(DIGIT_ROW), (u8)Y);
    lda8((u8)Y); ora8(0x10); wr8(ABS(DIGIT_ROW + 0x07), (u8)A);
    A = (A & 0xFF00) | pull8(); nz8((u8)A);
    ldy8(0x00);
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
    }
    push8((u8)A);
    wr8(ABS(DIGIT_ROW + 0x01), (u8)Y);
    lda8((u8)Y); ora8(0x10); wr8(ABS(DIGIT_ROW + 0x08), (u8)A);
    A = (A & 0xFF00) | pull8(); nz8((u8)A);
    wr8(ABS(DIGIT_ROW + 0x03), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x0A), (u8)A);
    lda8(rd8(DP(0x04)));
    and8(0x01);
    wr8(DP(0x04), (u8)A);
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    fc = 0; adc8(rd8(DP(0x04)));
    wr8(ABS(DIGIT_ROW + 0x04), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x0B), (u8)A);
    lda8(0xFF); wr8(ABS(DIGIT_ROW + 0x02), 0xFF);
    lda8(0xA6); wr8(ABS(DIGIT_ROW + 0x09), 0xA6);
    lda8(0xFF); wr8(ABS(DIGIT_ROW + 0x05), 0xFF);
    lda8(0xA4); wr8(ABS(DIGIT_ROW + 0x0C), 0xA4);
    lda8(0xFF); wr8(ABS(DIGIT_ROW + 0x06), 0xFF);
    lda8(0xA6); wr8(ABS(DIGIT_ROW + 0x0D), 0xA6);
    lda8(0x27); wr8(ABS(DIGIT_BUF), 0x27);
    VramQueueTilesLong_m1x1();
}
/* $80:9C87 (RTS): the entry screen: the entry code, $1923 and $0450 + 1;
 * the saved best total ($11:572C) at $0895 and best score ($11:5730) at
 * $08D6 in the font; the entry digits; with $81:800C set the row $80:9CCF
 * (palette $18) at $092B. */
void EntryScreenFill_m1x1(void)
{
    push8(0x80); push16(0x9C8A); ShowEntryCode_m1x1();
    wr8(ABS(FADE_STATE), inc8(rd8(ABS(FADE_STATE))));
    wr8(ABS(END_STEP), inc8(rd8(ABS(END_STEP))));
    rep(0x20);
    lda16(0x0895); wr16(DP(0x00), 0x0895);
    lda16(0x572C); wr16(DP(0x18), 0x572C);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x9CA6); QueueFontTime_m1x1();
    rep(0x20);
    lda16(0x08D6); wr16(DP(0x00), 0x08D6);
    lda16(0x5730); wr16(DP(0x18), 0x5730);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x9CBC); QueueFontDigits_m1x1();
    push8(0x80); push16(0x9CC0); QueueEntryDigits_m1x1();
    lda8(rd8(LONG(STADIUM_SELECT)));
    if (!fz) {
        rep(0x20);
        lda16(0x092B); wr16(DP(0x00), 0x092B);
        lda16(0x9CDD); wr16(DP(0x02), 0x9CDD);
        sep(0x20);
        lda8(0x18); wr8(DP(0x0F), 0x18);
        push16(0x9CDB); VramQueueTiles_m1x1();
    }
    ret();
}
/*
 * $81:F8BB BsxPollStream (M=1, X=1, RTL), once per frame from the main loop;
 * Stadium 1's routine with the minute counter at $B8.
 *
 * With a satellite receiver: when a packet is waiting on the opened time
 * channel and it is a clean clock packet, copy minute, hour and the other
 * clock bytes into the handshake area (restarting the frame counter when the
 * minute changes or on the first packet), then re-arm the stream.
 * Always: run the local clock. $7F:FFFA counts frames; after $0E4C (3660)
 * frames the minute advances (and $B8 counts it). At minute 60, unless the
 * hour is $11, write $80 to $2197 (end of broadcast: the BS-X unit takes over).
 */
void BsxPollStream_m1x1(void)
{
    push8(getP());
    sep(0x30);
    X = 0;
    if (rd8(ABS(BSX_STATUS)) != 0 && rd16(ABS(BSX_CHANNEL)) == rd16(LONG(BS_TIME_CHAN))) {
        if ((rd8(ABS(BSX_ERR)) & 0x0C) == 0) {
            u8 b = 0;
            for (int i = 0; i < 6; i++)                /* header: 5 skipped, 6th = 1 */
                b = rd8(ABS(BSX_DATA));
            int ok = b == 1 && rd8(ABS(BSX_DATA)) == 1;
            if (ok) {
                u8 z = rd8(ABS(BSX_DATA));
                z |= rd8(ABS(BSX_DATA));
                z |= rd8(ABS(BSX_DATA));
                ok = z == 0;
            }
            if (ok) {
                rd8(ABS(BSX_DATA));
                u8 minute = rd8(ABS(BSX_DATA));
                if (minute != rd8(LONG(BCAST_MINUTE)))
                    restart_minute4(0xF913, minute);
                wr8(LONG(BCAST_MINUTE), minute);
                wr8(LONG(BCAST_HOUR), rd8(ABS(BSX_DATA)));
                wr8(LONG(BSX_CLOCK_3), rd8(ABS(BSX_DATA)));
                wr8(LONG(BSX_CLOCK_2), rd8(ABS(BSX_DATA)));
                wr8(LONG(BSX_CLOCK_1), rd8(ABS(BSX_DATA)));
                rd8(ABS(BSX_DATA));
                u8 last = rd8(ABS(BSX_DATA));
                wr8(LONG(BSX_CLOCK_0), last);
                if (rd8(LONG(BSX_SYNCED)) == 0) {
                    restart_minute4(0xF946, last);
                    wr8(LONG(BSX_SYNCED), 1);
                }
            }
        }
        wr8(ABS(BSX_CTRL), 1);                     /* re-arm the stream */
        wr8(ABS(BSX_DATA), 1);
    }

    /* local clock */
    u16 frame = rd16(LONG(BCAST_FRAME));
    if (X == 0) {
        frame++;
        if (frame == 0x0E4C) {
            wr8(DP(0xB8), (u8)(rd8(DP(0xB8)) + 1));
            u8 minute = (u8)(rd8(LONG(BCAST_MINUTE)) + 1);
            wr8(LONG(BCAST_MINUTE), minute);
            if (minute == 0x3C && rd8(LONG(BCAST_HOUR)) != 0x11)
                wr8(ABS(BSX_RESET), 0x80);
            wr16(LONG(BSX_SYNCED), 0);
            frame = 0;
        }
    }
    wr16(LONG(BCAST_FRAME), frame);
    A = frame;
    setP(pull8());
    retl();
}
/* $81:FA81 BsxProbeReceiver (RTL, M=1 X=0): is a satellite receiver
 * there? $2194 = 1; $2188 must read back $AA and $55 as written; then
 * $2188/$2189/$218E/$218F are cleared and Z is set if they all read 0
 * (no receiver: Z clear). */
void BsxProbeReceiver_m1x0(void)
{
    lda8(0x01); wr8(ABS(0x2194), 0x01);
    lda8(0xAA); wr8(ABS(0x2188), 0xAA);
    cmp8((u8)A, rd8(ABS(0x2188)));
    if (!fz) { retl(); return; }
    lda8(0x55); wr8(ABS(0x2188), 0x55);
    cmp8((u8)A, rd8(ABS(0x2188)));
    if (!fz) { retl(); return; }
    wr8(ABS(0x2188), 0);
    wr8(ABS(0x2189), 0);
    wr8(ABS(0x218E), 0);
    wr8(ABS(0x218F), 0);
    lda8(rd8(ABS(0x2188)));
    ora8(rd8(ABS(0x2189)));
    ora8(rd8(ABS(0x218E)));
    ora8(rd8(ABS(0x218F)));
    retl();
}
/* $81:FA68 BsxDetect (RTL): $7F:FFE1 = 1 when a receiver answers the
 * probe, else 0; P kept. */
void BsxDetect_m1x1(void)
{
    push8(getP());
    sep(0x20);
    rep(0x10);
    lda8(0x00);
    wr8(LONG(NO_RECEIVER), 0);
    push8(0x81); push16(0xFA76); BsxProbeReceiver_m1x0();
    if (!fz) {
        lda8(0x01);
        wr8(LONG(NO_RECEIVER), 0x01);
    }
    setP(pull8());
    retl();
}
/* $81:F80C BsxOpenTimeChannel (RTL): the frame counter $7F:FFFA cleared;
 * look up the time file (type $0101, $13D3 / $13D5 = $0800); found: its
 * channel $13FE to $7F:FFF7 and $218E, the stream armed ($2191 / $2192),
 * carry clear; not found: $7F:FFF7 = $5533, $218E = $AAEE, carry set. P
 * otherwise kept. */
void BsxOpenTimeChannel_m1x1(void)
{
    push8(getP());
    rep(0x30);
    lda16(0x0000);
    wr16(LONG(BCAST_FRAME), 0);
    lda16(0x0101); wr16(ABS(0x13D3), 0x0101);
    lda16(0x0800); wr16(ABS(0x13D5), 0x0800);
    push8(0x81); push16(0xF825); BsxFindFile_m0x0();
    if (!fc) {
        lda16(rd16(ABS(0x13FE)));
        wr16(LONG(BS_TIME_CHAN), A);
        wr16(ABS(0x218E), A);
        lda16(0x0001);
        wr16(ABS(0x2191), 0x0001);
        wr16(ABS(0x2192), 0x0001);
        setP(pull8());
        fc = 0;
        retl();
        return;
    }
    rep(0x20);
    lda16(0x5533); wr16(LONG(BS_TIME_CHAN), 0x5533);
    lda16(0xAAEE); wr16(ABS(0x218E), 0xAAEE);
    setP(pull8());
    fc = 1;
    retl();
}
/* $82:9182 LoadScene (RTL, DB = $82 inside): load scene $76 with NMI and
 * HDMA off: $1566 from $82:92A7, the tilemaps cleared; then the scene's
 * list ($82:9323 + $82:92F1[$76], 8-byte entries: VRAM address, length,
 * source offset, packed data; $FFFF ends), each entry unpacked to $7E:4000
 * and DMAed to VRAM. Scene 4 in a race, and scenes 9 / 10 before minute 40,
 * use the alternative list (+$24 / +$2C / +$24). After the list, the
 * palette data unpacked to low RAM, $1915 + 1, $190F and $1E0C from
 * $82:92D9 / $92C0, $1B00-$1B0F and $1B20-$1B2F cleared, NMI back on; X
 * and Y kept. */
void LoadScene_m1x1(void)
{
    push8(DB);
    lda8(0x82);
    push8(0x82); DB = pull8(); nz8(DB);
    push8((u8)X); push8((u8)Y);
    X = rd8(DP(0x76)); nz8((u8)X);
    lda8(rd8(ABSX(0x92A7))); wr8(ABS(0x1566), (u8)A);
    wr8(ABS(NMITIMEN), 0);
    wr8(ABS(SH_HDMAEN), 0);
    wr8(ABS(HDMAEN), 0);
    push8(0x82); push16(0x919D); ClearTilemaps_m1x1();
    rep(0x30);
    lda16(rd16(ABS(GAME_MODE))); and16(0x00FF);
    lda16(asl16(A));
    X = A; nz16(X);
    Y = rd16(ABSX(0x92F1)); nz16(Y);
    lda16(rd16(ABS(GAME_MODE))); and16(0x00FF);
    u16 extra = 0;
    cmp16(A, 0x0004);
    if (fz) {
        lda16(rd16(ABS(FINAL_RACE))); and16(0x00FF);
        if (fz) extra = 0x0024;
    } else {
        cmp16(A, 0x0009);
        int nine = fz;
        if (!nine) cmp16(A, 0x000A);
        if (nine || fz) {
            sep(0x20);
            lda8(rd8(LONG(BCAST_MINUTE)));
            rep(0x20);
            and16(0x00FF);
            cmp16(A, 0x0028);
            if (!fc) extra = nine ? 0x002C : 0x0024;
        }
    }
    if (extra) {
        lda16(Y);
        fc = 0; adc16(extra);
        Y = A; nz16(Y);
    }
    for (;;) {
        WAIT_TICK();
        lda16(rd16(ABSY(0x9323)));
        cmp16(A, 0xFFFF);
        if (fz) break;
        wr16(ABS(0x0102), A);
        lda16(rd16(ABSY(0x9325))); wr16(ABS(0x0100), A);
        lda16(rd16(ABSY(0x9327))); wr16(ABS(0x0104), A);
        lda16(rd16(ABSY(0x9329))); wr16(DP(0x00), A);
        push16(Y);
        sep(0x30);
        push16(0x921F); UnpackToWram_m1x1();
        rep(0x20);
        push16(D);
        lda16(0x4300);
        D = A;
        ldy8(0x80); wr8(ABS(VMAIN), 0x80);
        lda16(0x1801); wr16(DP(0x00), 0x1801);
        lda16(rd16(ABS(0x0102))); wr16(ABS(VMADDL), A);
        lda16(rd16(ABS(0x0100))); wr16(DP(0x05), A);
        lda16(0x4000);
        fc = 0; adc16(rd16(ABS(0x0104)));
        ldx8(0x7E);
        sep(0x10);
        ldy8(0x01);
        wr16(DP(0x02), A);
        wr8(DP(0x04), (u8)X);
        wr8(ABS(MDMAEN), (u8)Y);
        D = pull16(); nz16(D);
        rep(0x10);
        Y = pull16(); nz16(Y);
        lda16(Y);
        fc = 0; adc16(0x0008);
        Y = A; nz16(Y);
    }
    Y = inc16(Y);
    Y = inc16(Y);
    lda16(rd16(ABSY(0x9323))); wr16(DP(0x00), A);
    sep(0x30);
    push16(0x9267); UnpackToLowRam_m1x1();
    wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
    Y = rd8(DP(0x76)); nz8((u8)Y);
    lda8(rd8(ABSY(0x92D9))); wr8(ABS(SH_HDMAEN), (u8)A);
    lda8(rd8(ABSY(0x92C0))); wr8(ABS(ANIM_UPLOAD_ON), (u8)A);
    ldx8(0x0F);
    do {
        wr8(ABSX(0x1B00), 0);
        wr8(ABSX(0x1B20), 0);
        X = dec8((u8)X);
    } while (!fn);
    lda8(rd8(ABS(SH_NMITIMEN))); wr8(ABS(NMITIMEN), (u8)A);
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    DB = pull8(); nz8(DB);
    retl();
}
/* $81:A0F4 LoadCourse (RTL): unpack course $7C (stadium $81:800C; the
 * attract demo uses entry 8) into the track buffer $7F:4000. Kept when
 * the buffer is already filled in a free run ($7D). The list at $81:8C8A
 * (from $81:8C62, length $81:8C76) holds piece bytes; a byte with bit 7
 * is followed by a repeat count, piece 9 (a jump) fills six columns. In
 * mode 3 a random jump past a random distance gets bonus piece $8B / $8C.
 * The port applies custom courses inside this routine (port/src/courses.c),
 * so it stays recompiled there. */
void LoadCourse_m1x1(void)
{
    rep(0x30);
    lda16(rd16(LONG(TRACK_BUF)));
    if (!fz) {
        lda16(rd16(DP(FINAL_RACE))); and16(0x00FF);
        if (!fz) { sep(0x30); retl(); return; }
    }
    X = 0x0FFE; nz16(X);
    lda16(0x0000);
    do {
        wr16(LONGX(TRACK_BUF), 0);
        X = dec16(X); X = dec16(X);
    } while (!fn);
    lda16(rd16(ABS(0x0CD7)));
    if (!fz) {
        X = 0x0010; nz16(X);
    } else {
        lda16(rd16(DP(COURSE_SLOT))); and16(0x00FF);
        wr16(DP(0x00), A);
        lda16(rd16(LONG(STADIUM_SELECT)));
        lda16(asl16(A)); lda16(asl16(A));
        fc = 0; adc16(rd16(DP(0x00)));
        lda16(asl16(A));
        X = A; nz16(X);
    }
    lda16(rd16(LONGX(0x818C76))); wr16(DP(0x00), A);
    lda16(rd16(LONGX(0x818C62)));
    X = A; nz16(X);
    and16(0x00FF);
    sep(0x20);
    Y = 0x0000; nz16(Y);
    push8(DB);
    lda8(0x7F);
    push8(0x7F); DB = pull8(); nz8(DB);
    do {
        WAIT_TICK();
        lda8(rd8(LONGX(0x818C8A)));
        wr8(ABSY(0x4000), (u8)A);
        cmp8((u8)A, 0x09);
        if (fz) {
            for (int k = 1; k <= 5; k++) wr8(ABSY(0x4000 + k), (u8)A);
        }
        fz = (A & 0x80) == 0;
        if (!fz) {
            wr8(DP(0x02), (u8)A);
            rep(0x20);
            wr16(DP(0x00), dec16(rd16(DP(0x00))));
            sep(0x20);
            X = inc16(X);
            lda8(rd8(LONGX(0x818C8A)));
            lda8(dec8((u8)A));
            push16(X);
            X = A; nz16(X);
            do {
                WAIT_TICK();
                lda8(rd8(DP(0x02)));
                wr8(ABSY(0x4000), (u8)A);
                Y = inc16(Y);
                X = dec16(X);
            } while (!fn);
            X = pull16(); nz16(X);
            Y = dec16(Y);
        }
        X = inc16(X);
        Y = inc16(Y);
        rep(0x20);
        wr16(DP(0x00), dec16(rd16(DP(0x00))));
        sep(0x20);
    } while (!fz);
    /* the port's custom and Stadium 1-2 courses (roster.h; DB is $7F here) */
    port_course_override(rd16(LONG(0x7E0CD7)) ? 8 : rd8(LONG(STADIUM_SELECT)) * 4 + rd8(DP(COURSE_SLOT)));
    lda8(rd8(DP(GAME_MODE)));
    cmp8((u8)A, 0x03);
    if (fz) {
        push8(0x81); push16(0xA194); RandomB_m1x0();
        and8(0x03);
        A = (u16)((A << 8) | (A >> 8)); nz8((u8)A);       /* XBA */
        push8(0x81); push16(0xA19B); RandomB_m1x0();
        Y = A; nz16(Y);
        X = 0x0000; nz16(X);
        for (;;) {
            do {
                WAIT_TICK();
                lda8(rd8(LONGX(TRACK_BUF)));
                cmp8((u8)A, 0x09);
                if (fz) { X = 0x0000; nz16(X); }
                X = inc16(X);
                Y = dec16(Y);
            } while (!fz);
            cmp8((u8)A, 0x80);
            if (fz) break;
            X = inc16(X);
            Y = inc16(Y);
        }
        push8(0x81); push16(0xA1BA); RandomB_m1x0();
        and8(0x01);
        fc = 0; adc8(0x8B);
        wr8(LONGX(0x7F3FFF), (u8)A);
    }
    sep(0x10);
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:B390 QueueTileBlockLong (RTL): QueueTileBlock for other banks. */
void QueueTileBlockLong_m1x1(void)
{
    push16(0xB392); QueueTileBlock_m1x1();
    retl();
}
/* $80:F233 (RTS): the attract demo race: course 7, character 1 with its
 * stadium palette, the riders' start state, the course loaded and the pad
 * display. */
void StartDemoRace_m1x1(void)
{
    lda8(0x07); wr8(DP(COURSE_SLOT), 0x07);
    lda8(0x01); wr8(ABS(PLAYER_CHAR), 0x01);
    push8(0x80); push16(0xF23F); LoadStadiumPalette_m1x1();
    course_start_riders4(0);
    push8(0x80); push16(0xF29A); CALL(LoadCourse_m1x1, 0x81A0F4, 3);
    push16(0xF29D); DemoPadDisplay_m1x1();
    ret();
}
/* $81:C1AE (RTL): the course-select preview: BG mode 9, the riders' start
 * state, the course number label (tiles $C0 + 2 * $7C, palette $42 / $19)
 * at VRAM $088D, $88 / $1C00 / $0CD7 / $197F / $0928 / $190D cleared,
 * $1915 + 1, the rider palettes and the course loaded. */
void CourseSelectPreview_m1x1(void)
{
    lda8(0x09); wr8(ABS(BGMODE), 0x09);
    lda8(0x01); wr8(ABS(BG1SC), 0x01);
    course_start_riders4(1);
    lda8(0x42); wr8(ABS(DIGIT_BUF), 0x42);
    lda8(0x19);
    for (int k = 0; k < 8; k++) wr8(ABS(DIGIT_ROW + 0x01 + 2 * k), 0x19);
    lda8(rd8(DP(COURSE_SLOT)));
    lda8(asl8((u8)A));
    ora8(0xC0);
    for (int k = 0; k < 4; k++) {
        if (k) { fc = 0; adc8(0x0F); }
        wr8(ABS(DIGIT_ROW + 4 * k), (u8)A);
        lda8(inc8((u8)A));
        wr8(ABS(DIGIT_ROW + 0x02 + 4 * k), (u8)A);
    }
    rep(0x20);
    lda16(0x088D); wr16(DP(0x00), 0x088D);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x18); wr8(DP(0x0F), 0x18);
    push8(0x81); push16(0xC278); QueueTileBlockLong_m1x1();
    rep(0x20);
    wr16(DP(MODE_TIMER), 0);
    wr16(ABS(PALETTE_BUF), 0);
    sep(0x20);
    wr8(ABS(ATTRACT_DEMO), 0);
    wr8(ABS(JUMP_RUN_STEP), 0);
    wr8(ABS(LAP), 0);
    wr8(ABS(SH_CGADSUB), 0);
    wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
    push8(0x81); push16(0xC294); LoadRiderPalettes_m1x1();
    push8(0x81); push16(0xC298); CALL(LoadCourse_m1x1, 0x81A0F4, 3);
    retl();
}
/* $80:A862 RacePlaceRiders (RTS): set up a race: $1923 = 1, $75 + 1, $54
 * (default 5), $0CD5 = $A0, lanes $0936-$093D; the riders (the player
 * only, or all eight in a free run) from $80:B854 / $B84C / $B85C, their
 * pieces and speeds; one rival's lane piece halved; ground heights and the
 * sorted riders; in a race the clock $083A = 4:00 and the score copied to
 * $1549-$154E; the best time and race time at $1897; the course loaded;
 * $0819-$0836 = $FF; markers, start flags, $A6 = 4; the bonus order
 * $0D01-$0D10 from $80:A9B1 shuffled 16 times; in a race $1542-$1548
 * cleared; the coin rank, rider palettes, $1905 = $17. */
void RacePlaceRiders_m1x1(void)
{
    lda8(0x01); wr8(ABS(FADE_STATE), 0x01);
    wr8(DP(MODE_ACTIVE), inc8(rd8(DP(MODE_ACTIVE))));
    lda8(rd8(DP(0x54)));
    if (fz) lda8(0x05);
    wr8(DP(0x54), (u8)A);
    lda8(0xA0); wr8(ABS(GROUND_LINE), 0xA0);
    ldx8(0x07);
    lda8(0xC0);
    do {
        wr8(ABSX(0x0936), (u8)A);
        fc = 1; sbc8(0x10);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0xFF); wr8(ABS(0x098B), 0xFF);
    ldx8(0x01);
    wr8(DP(0x94), 0x01);
    wr8(ABS(0x08C4), 0x01);
    X = inc8((u8)X);
    wr8(ABS(0x098C), (u8)X);
    X = rd8(DP(FINAL_RACE)); nz8((u8)X);
    if (!fz) ldx8(0x07);
    do {
        lda8(rd8(ABSX(0xB85D))); wr8(ABSX(RIDER_LANE), (u8)A);
        lda8(0x08); wr8(ABSX(RIDER_WHEEL), 0x08);
        lda8(rd8(ABSX(0xB855))); wr8(ABSX(RIDER_COLUMN), (u8)A);
        lda8(rd8(ABSX(0xB865))); wr8(ABSX(RIDER_X), (u8)A);
        lda8(0x06); wr8(ABSX(RIDER_LEAN), 0x06); wr8(ABSX(RIDER_TARGET_HI), 0x06);
        lda8(0x01); wr8(ABSX(RIDER_PAGE), 0x01); wr8(ABSX(RIDER_ACTIVE), 0x01);
        lda8(rd8(DPX(0x2D)));
        lda8(lsr8((u8)A));
        wr8(ABSX(RIDER_TARGET_LO), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    lda8(rd8(DPX(0x2D)));
    and8(0x03);
    if (fz) lda8(0x02);
    X = (u8)A; nz8((u8)X);
    wr8(ABSX(RIDER_TARGET_HI), lsr8(rd8(ABSX(RIDER_TARGET_HI))));
    push16(0xA8D7); RiderGroundHeights_m1x1();
    push16(0xA8DA); SortAndDrawRiders_m1x1();
    lda8(rd8(DP(FINAL_RACE)));
    if (fz) {
        lda8(0x04); wr8(ABS(RECORD_TIME), 0x04);
        wr8(ABS(RECORD_TIME + 0x01), 0);
        wr8(ABS(RECORD_TIME + 0x02), 0);
        for (int k = 0; k < 6; k++) {
            lda8(rd8(ABS(SCORE_DIGITS + k)));
            wr8(ABS(SCORE_COPY + k), (u8)A);
        }
    }
    wr8(ABS(RECORD_COURSE), 0);
    push8(0x80); push16(0xA914); ShowCourseBest_m1x1();
    lda8(0x97); wr8(DP(0x00), 0x97);
    lda8(0x18); wr8(DP(0x01), 0x18);
    push16(0xA91F); QueueRaceTime_m1x1();
    push8(0x80); push16(0xA923); CALL(LoadCourse_m1x1, 0x81A0F4, 3);
    push8(0x80); push16(0xA927); ClearWords1530_m1x1();
    lda8(rd8(DP(FINAL_RACE)));
    if (fz) {
        wr8(ABS(TURBO_ON), 0);
        wr8(ABS(COIN_COUNT), 0);
        wr8(ABS(COIN_COMBO), 0);
        wr8(ABS(MARKER_COUNT), 0);
    }
    lda8(rd8(ABS(TURBO_ON)));
    if (!fz) { lda8(0x20); wr8(ABS(ENGINE_HEAT), 0x20); }
    ldx8(0x1D);
    lda8(0xFF);
    do {
        wr8(ABSX(0x0819), 0xFF);
        X = dec8((u8)X);
    } while (!fn);
    wr8(ABS(SH_CGADSUB), 0);
    push8(0x80); push16(0xA952); MarkerObjects_m1x1();
    ldy8(0xE1);
    push16(0xA957); DrawStartFlags_m1x1();
    lda8(0x04); wr8(ABS(PIC_SCROLL), 0x04);
    ldx8(0x0E);
    do {
        lda8(rd8(ABSX(0xA9BA))); wr8(ABSX(0x0D01), (u8)A);
        lda8(rd8(ABSX(0xA9BB))); wr8(ABSX(0x0D02), (u8)A);
        X = dec8((u8)X); X = dec8((u8)X);
    } while (!fn);
    lda8(0x10); wr8(DP(0x00), 0x10);
    do {
        push8(0x80); push16(0xA976); RandomB_m1x1();
        and8(0x0E);
        Y = (u8)A; nz8((u8)Y);
        push8(0x80); push16(0xA97D); RandomB_m1x1();
        and8(0x0E);
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(0x0D01)));
        push8((u8)A);
        lda8(rd8(ABSY(0x0D01))); wr8(ABSX(0x0D01), (u8)A);
        A = (A & 0xFF00) | pull8(); nz8((u8)A);
        wr8(ABSY(0x0D01), (u8)A);
        wr8(DP(0x00), dec8(rd8(DP(0x00))));
    } while (!fz);
    lda8(rd8(ABS(FINAL_RACE)));
    if (fz) {
        wr8(ABS(0x1544), 0); wr8(ABS(0x1545), 0); wr8(ABS(0x1546), 0);
        wr8(ABS(0x1547), 0); wr8(ABS(0x1548), 0);
        wr8(ABS(0x1543), 0); wr8(ABS(COIN_TOTAL), 0);
    }
    push16(0xA9AF); ShowCoinRank_m1x1();
    push8(0x80); push16(0xA9B3); LoadRiderPalettes_m1x1();
    lda8(0x17); wr8(ABS(SH_TM), 0x17);
    ret();
}
/* $80:9DE3 (RTS): start the closing credits: with NMI off the credits
 * music (stream 0x06) uploaded, NMI back, $75 + 1; BG3 / tile
 * bases; scroll $A6 = $FFFF, colour math $1C00 = $90 / $1C01 =
 * $20, $1915 + 1; sound command $10; HDMA channel 7 (direct, to $2100,
 * table $00:0452) with the table copied from $80:9E5E;
 * $190F = $80, $1905 = 4, $1906 cleared, BG mode 9, $087E / $0880 / $0882 / $88
 * cleared. */
void StartCredits_m1x1(void)
{
    wr8(ABS(NMITIMEN), 0);
    ldx8(0x06);
    push8(0x80); push16(0x9DEB); SpcUploadB_m1x1();
    lda8(rd8(ABS(SH_NMITIMEN))); wr8(ABS(NMITIMEN), (u8)A);
    wr8(DP(MODE_ACTIVE), inc8(rd8(DP(MODE_ACTIVE))));
    lda8(0x32); wr8(ABS(BG3SC), 0x32);
    lda8(0x44); wr8(ABS(BG12NBA), 0x44);
    lda8(0x01); wr8(ABS(BG34NBA), 0x01);
    rep(0x20);
    lda16(0xFFFF); wr16(DP(PIC_SCROLL), 0xFFFF);
    lda16(0x0090); wr16(ABS(PALETTE_BUF), 0x0090);
    lda16(0x0020); wr16(ABS(PALETTE_BUF + 0x01), 0x0020);
    sep(0x20);
    wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
    lda8(0x10); wr8(ABS(APUIO0), 0x10);
    lda8(0x00); wr8(ABS(DMAP7), 0);
    lda8(0x00); wr8(ABS(BBAD7), 0);
    lda8(0x52); wr8(ABS(A1T7L), 0x52);
    lda8(0x04); wr8(ABS(A1T7H), 0x04);
    lda8(0x00); wr8(ABS(A1B7), 0);
    lda8(0x00); wr8(ABS(DASB7), 0);
    ldx8(0x40);
    do {
        lda8(rd8(ABSX(0x9E67))); wr8(ABSX(0x0452), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x80); wr8(ABS(SH_HDMAEN), 0x80);
    lda8(0x04); wr8(ABS(SH_TM), 0x04);
        wr8(ABS(SH_TS), 0);
    lda8(0x09); wr8(ABS(BGMODE), 0x09);
    wr8(ABS(BAR_LENGTH), 0);
    wr8(ABS(COLOUR_CYCLE), 0);
    wr8(ABS(COLOUR_TIMER), 0);
    wr8(DP(MODE_TIMER), 0);
    ret();
}
/* Empty routines (RTS): $80:BB4E, $80:F03A, $81:9A26, $81:B928. */
void EmptyBB45_m1x1(void) { ret(); }
void EmptyF03A_m1x1(void) { ret(); }
void Empty9A26_m1x1(void) { ret(); }
void EmptyB928_m1x1(void) { ret(); }
/* $80:B566 ModeNop (RTS). */
void ModeNop_m1x1(void) { ret(); }
/* $80:86DF (RTS): next mode step ($75 + 1). */
void ModeStepNext_m1x1(void)
{
    wr8(DP(MODE_ACTIVE), inc8(rd8(DP(MODE_ACTIVE))));
    ret();
}
/* $80:EA05 (RTS): the jump contest total (JumpResultTotal). */
void JumpTotalStep_m1x1(void)
{
    push8(0x80); push16(0xEA08); JumpResultTotal_m1x1();
    ret();
}
/* $80:FB71 / $80:FBA9 / $80:FB9A (RTS by tail, X = rider): jump base $30
 * or $10; the last also sets $0206,X = 2 and $0246,X = 1. */
void JumpBase30_m1x1(void) { lda8(0x30); SetJumpBase_m1x1(); }
void JumpBase10_m1x1(void) { lda8(0x10); SetJumpBase_m1x1(); }
void StartJumpBase30_m1x1(void)
{
    lda8(0x02); wr8(ABSX(RIDER_LANDING), 0x02);
    lda8(0x01); wr8(ABSX(RIDER_SLOW_AT), 0x01);
    JumpBase30_m1x1();
}
/* $81:C011 (RTS): clear the frame $034F of object $03A3. */
void ClearObjectFrame_m1x1(void)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    wr8(ABSX(OBJ_FRAME), 0);
    ret();
}
/* $80:8F9B (RTS): the HDMA channels, $95 = 1. */
void SetupHdmaFlag_m1x1(void)
{
    push8(0x80); push16(0x8F9E); SetupHdmaChannels_m1x1();
    lda8(0x01); wr8(DP(HDMA_ZOOM), 0x01);
    ret();
}
/* $80:EFFF (RTS by tail): step $C7 through $80:F034. */
void DemoStepDispatch_m1x1(void) { table_jump4(0x80F006, rd8(DP(JUMP_STATE)), 0x80F031); }
/* $81:95F4 (RTS by tail, Y = object): step $012E,Y through $81:95FC. */
void ObjectStepDispatch_m1x1(void) { table_jump4(0x8195FC, rd8(ABSY(EFFECT_STEP)), 0x8195F9); }
void ObjectState9835_m1x1(void) { object_state_jump4(0x819840, 0x81983D); }
void ObjectStateB8E3_m1x1(void) { object_state_jump4(0x81B8EE, 0x81B8EB); }
void ObjectStateB95D_m1x1(void) { object_state_jump4(0x81B968, 0x81B965); }
void ObjectStateBA4C_m1x1(void) { object_state_jump4(0x81BA57, 0x81BA54); }
void ObjectStateBB80_m1x1(void) { object_state_jump4(0x81BB8B, 0x81BB88); }
void ObjectStateBCC1_m1x1(void) { object_state_jump4(0x81BCCC, 0x81BCC9); }
void ObjectStateBDDC_m1x1(void) { object_state_jump4(0x81BDE7, 0x81BDE4); }
void ObjectStateBF34_m1x1(void) { object_state_jump4(0x81BF3F, 0x81BF3C); }
void ObjectStateBFFC_m1x1(void) { object_state_jump4(0x81C007, 0x81C004); }
/* $80:F91F (RTS by tail, X = rider): the rider pair for 2, halved, then
 * HeightBelow. */
void PairHeight2_m1x1(void)
{
    lda8(0x02);
    push16(0xF923); SetRiderPair_m1x1();
    lda8(lsr8((u8)A));
    HeightBelow_m1x1();
}
/* $80:F9C2 (RTS, X = rider): clear $020E,X and $0206,X. */
void ClearJumpFlags_m1x1(void)
{
    lda8(0x00);
    wr8(ABSX(RIDER_JUMP), 0);
    set_jump_state4();
}
/* $80:FB20 (RTS, X = rider): $0206,X = 1. */
void SetJumpState1_m1x1(void)
{
    lda8(0x01);
    set_jump_state4();
}
/* $81:C3A5 (RTS): past this course's minute ($81:C3A1 by $7C) of the
 * broadcast, go to mode 3. */
void CourseMinuteCheck_m1x1(void)
{
    X = rd8(DP(COURSE_SLOT)); nz8((u8)X);
    lda8(rd8(LONG(BCAST_MINUTE)));
    cmp8((u8)A, rd8(LONG(0x81C3A1 + X)));
    if (fc) {
        lda8(0x03); wr8(DP(GAME_MODE), 0x03);
        push8(0x81); push16(0xC3B8); ChangeModeLong_m1x1();
    }
    ret();
}
/* $80:F228 (RTS): BG mode 9, BG1SC = 1. */
void SetRaceBgMode_m1x1(void)
{
    lda8(0x09); wr8(ABS(BGMODE), 0x09);
    lda8(0x01); wr8(ABS(BG1SC), 0x01);
    ret();
}
/* $80:F37A (RTS by tail): rival 7 ($092A = 7, $019E,7 cleared) on lane 3. */
void PlaceLastRival_m1x1(void)
{
    ldx8(0x07);
    wr8(ABS(0x092A), 0x07);
    wr8(ABSX(RIDER_PAGE), 0);
    ldy8(0x03);
    PlaceRival_m1x1();
}
/* $80:F90D (RTS, X = rider): $0246,X = $80; the rider pair for 5, halved,
 * into $01FE,X and $026E,X. */
void PairHeight5_m1x1(void)
{
    lda8(0x80); wr8(ABSX(RIDER_SLOW_AT), 0x80);
    lda8(0x05);
    push16(0xF916); SetRiderPair_m1x1();
    lda8(lsr8((u8)A));
    wr8(ABSX(RIDER_HEIGHT), (u8)A);
    wr8(ABSX(RIDER_JUMP_BASE), (u8)A);
    ret();
}
/* $80:FC20 (RTS, X = rider): clear $0226,X, $01A6,X and $0981,X. */
void ClearRiderAction_m1x1(void)
{
    lda8(0x00);
    wr8(ABSX(RIDER_PIECE_ACTION), 0);
    wr8(ABSX(RIDER_POSE), 0);
    wr8(ABSX(0x0981), 0);
    ret();
}
/* $80:F928 (RTS, X = rider): the rider pair for 6, the jump base, $0246,X
 * = $60. */
void PairJump6_m1x1(void)
{
    lda8(0x06);
    push16(0xF92C); SetRiderPair_m1x1();
    push16(0xF92F); SetJumpBase_m1x1();
    lda8(0x60); wr8(ABSX(RIDER_SLOW_AT), 0x60);
    ret();
}
/* $80:A637 (RTS by tail): the tile block $80:A63F at VRAM $18D2. */
void QueueStadiumLabel_m1x1(void)
{
    rep(0x20);
    lda16(0x18D2); wr16(DP(0x00), 0x18D2);
    lda16(0xA648); wr16(DP(0x02), 0xA648);
    sep(0x20);
    QueueTileBlock_m1x1();
}
/* $80:8FEF (RTS): stadium 1's results row, badge and label ($0928 = 1,
 * then cleared). */
void ShowStadium1Row_m1x1(void)
{
    lda8(0x01); wr8(ABS(LAP), 0x01);
    push16(0x8FF6); QueueResultsRow_m1x1();
    push8(0x80); push16(0x8FFA); QueueStadiumBadge_m1x1();
    push16(0x8FFD); QueueStadiumLabel_m1x1();
    wr8(ABS(LAP), 0);
    ret();
}
/* $80:F00D (RTS): $1569 = $9C - $C8. */
void ScrollDelta_m1x1(void)
{
    rep(0x20);
    lda16(rd16(DP(SCROLL_X)));
    fc = 1; sbc16(rd16(DP(0xC8)));
    wr16(ABS(JUMP_COUNT), A);
    sep(0x20);
    ret();
}
/* $80:F21A (RTS): course 0, race, $7E cleared, attract demo $0CD7 = $0F. */
void ResetDemoCourse_m1x1(void)
{
    lda8(0x00);
    wr8(DP(COURSE_SLOT), 0); wr8(DP(FINAL_RACE), 0); wr8(DP(0x7E), 0);
    lda8(0x0F); wr8(ABS(ATTRACT_DEMO), 0x0F);
    ret();
}
/* $80:F9CB (RTS, X = rider): $020E,X = 1, or 2 when $01F6,X < $20. */
void SetJumpKind_m1x1(void)
{
    lda8(0x01); wr8(ABSX(RIDER_JUMP), 0x01);
    lda8(rd8(ABSX(RIDER_LANE)));
    cmp8((u8)A, 0x20);
    if (!fc) wr8(ABSX(RIDER_JUMP), inc8(rd8(ABSX(RIDER_JUMP))));
    ret();
}
/* $80:91F2 (RTS by tail): the race HUD unless in the attract demo, then
 * ResetRaceBoard. */
void StartRaceBoard_m1x1(void)
{
    lda8(rd8(ABS(ATTRACT_DEMO)));
    if (fz) { push16(0x91F9); DrawRaceHud_m1x1(); }
    ResetRaceBoard_m1x1();
}
/* Demo exits that continue with the next entry: $80:F36F ($099D = $0990
 * = $10), $80:F3BA and $80:F3BE (A = $35 + $80:D200,X, unused). */
void DemoExitBoost_m1x1(void)
{
    lda8(0x10);
    wr8(ABS(0x099D), 0x10);
    wr8(ABS(ENGINE_HEAT), 0x10);
    demo_next_entry4();
}
void DemoExitSkip40_m1x1(void)
{
    lda8(0x40);
    demo_next_entry4();
}
void DemoExitSkip35_m1x1(void)
{
    lda8(0x35);
    fc = 0; adc8(rd8(ABSX(0xD200)));
    demo_next_entry4();
}
/* $81:C641 / $81:C62B / $81:C636 / $81:C643 (RTS): the engine pitch $198A
 * unchanged, + 4, + 8 or - 8, sent to APU port 2. */
void PitchHold_m1x1(void) { send_pitch4(); }
void PitchUp4_m1x1(void)
{
    lda8(rd8(ABS(ENGINE_PITCH)));
    fc = 0; adc8(0x04);
    wr8(ABS(ENGINE_PITCH), (u8)A);
    send_pitch4();
}
void PitchUp8_m1x1(void)
{
    lda8(rd8(ABS(ENGINE_PITCH)));
    fc = 0; adc8(0x08);
    wr8(ABS(ENGINE_PITCH), (u8)A);
    send_pitch4();
}
void PitchDown8_m1x1(void)
{
    lda8(rd8(ABS(ENGINE_PITCH)));
    fc = 1; sbc8(0x08);
    wr8(ABS(ENGINE_PITCH), (u8)A);
    send_pitch4();
}
/* $81:C393 (RTS): with no engine pitch, $197F + 1; else the jump rider
 * step. */
void JumpPitchStep_m1x1(void)
{
    lda8(rd8(ABS(ENGINE_PITCH)));
    if (fz) wr8(ABS(JUMP_RUN_STEP), inc8(rd8(ABS(JUMP_RUN_STEP))));
    else { push16(0xC39F); JumpRiderStep_m1x1(); }
    ret();
}
/* $80:D7A6 (RTS): $1958 + 1, $B4 = $1E, $B5 = 1, $1990 cleared, then the
 * bonus count. */
void StartBonusCount_m1x1(void)
{
    wr8(ABS(SPRITES_FROZEN), inc8(rd8(ABS(SPRITES_FROZEN))));
    lda8(0x1E); wr8(DP(BONUS_SECONDS), 0x1E);
    lda8(0x01); wr8(DP(BONUS_TICK), 0x01);
    wr8(ABS(BONUS_END), 0);
    QueueBonusCount_m1x1();
}
/* $81:C7E0 (RTL): the jump contest result frame: riders drawn, $1961 =
 * $19; when the delay $8C runs out, the jump distance. */
void JumpDistanceStep_m1x1(void)
{
    ldx8(0x00);
    push8(0x81); push16(0xC7E5); DrawAllRidersLong_m1x1();
    lda8(0x19); wr8(ABS(SPRITE_FRAME), 0x19);
    wr8(DP(TIMER_B), dec8(rd8(DP(TIMER_B))));
    if (!fz) { retl(); return; }
    JumpResultDistance_m1x1();
}
/* $80:E93C (RTS): JumpDistanceStep. */
void JumpDistanceStepNear_m1x1(void)
{
    push8(0x80); push16(0xE93F); JumpDistanceStep_m1x1();
    ret();
}
/* $80:FB25 (RTS, X = rider): on the ground a bump crash: unless already
 * crashed ($01CE), the player gets sound $3B, and the speed $01C6 grows
 * (to 4, then $01BE = $C0); the crash starts at speed 0. */
void RiderBumpCrash_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_AIR)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_BUMPED)));
    if (fz) {
        cmp8((u8)X, 0x00);
        if (fz) {
            lda8(0x3B);
            push8(0x80); push16(0xC0D8); QueueSoundB_m1x1();
        }
        lda8(rd8(ABSX(RIDER_SPEED_HI)));
        cmp8((u8)A, 0x04);
        if (!fc) wr8(ABSX(RIDER_SPEED_HI), inc8(rd8(ABSX(RIDER_SPEED_HI))));
        else { lda8(0xC0); wr8(ABSX(RIDER_SPEED_LO), 0xC0); }
    }
    lda8(0x00);
    crash_from();
    ret();
}
/* $80:FC02 (RTS, X = rider): by the jump state $020E: 1 = landed ($01FE,
 * $09C4, $09C7 cleared, then CrashLand), 3 = the jump flags cleared. */
void LandingStep_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_JUMP)));
    cmp8((u8)A, 0x01);
    if (fz) {
        lda8(0x00); wr8(ABSX(RIDER_HEIGHT), 0);
        lda8(0x00);
        wr8(ABS(PLAYER_GROUND), 0);
        wr8(ABS(0x09C7), 0);
        CrashLand_m1x1();
        return;
    }
    cmp8((u8)A, 0x03);
    if (!fz) { ret(); return; }
    ClearJumpFlags_m1x1();
}
/* $80:F81D PieceAction (RTS, A = action 0-26): jump through the rider
 * piece action table $80:F6DF. Checked with difftest --set-a for each
 * action (higher numbers read past the table). */
void PieceAction_m1x1(void)
{
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xF6AC))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSY(0xF6AD))); wr8(DP(0x01), (u8)A);
    if (!r816_call_pc(0x800000u | rd16(LONG(0x000000))))
        r816_unknown_target(0x80F85C, rd16(LONG(0x000000)));
}
/* $80:FFA0 (RTI): the empty handler for COP, BRK and ABORT. */
void EmptyVector_m1x1(void) { rti4(); }
/* $80:B91B with X = 8-bit (from Irq, D = $1B00): as ScaleByScroll_m0x0. */
void ScaleByScroll_m0x1(void)
{
    wr16(DP(0x62), A);
    sep(0x20);
    push16(0xB921); Mul16b_m1x0();
    rep(0x20);
    lda16(rd16(DP(0x65)));
    ret();
}
/* $80:FFA1 -> $80:8256 Nmi (RTI): V timer line $AC with the parallax ($96)
 * else 6; forced blank and HDMA off; unless the frame was already serviced
 * ($0817), the VRAM uploads, the VRAM queue and the BG1-BG3 scroll (BG1 V
 * scroll $9E/$9F with $94, else $9B and a wobble from $90 bit 1); NMI/IRQ
 * enable (V timer with $95 or $96), screen, layer, HDMA and colour math
 * registers from $1903-$190F; the four sound command channels (channel 3
 * spins until its port echoes); $0817 + 1. */
void NmiVector_m1x1(void)
{
    rep(0x30);
    push16(A); push16(X); push16(Y); push16(D);
    lda16(0x0000); D = A; nz16(D);
    push8(DB); push8(0x80); DB = pull8(); nz8(DB);
    lda16(rd16(ABS(PARALLAX_ON)));
    lda16(fz ? 0x0006 : 0x00AC);
    wr16(ABS(VTIMEL), A);
    sep(0x30);
    lda8(rd8(ABS(RDNMI)));
    lda8(0x80); wr8(ABS(INIDISP), 0x80);
    wr8(ABS(HDMAEN), 0);
    lda8(rd8(ABS(NMI_FRAME_READY)));
    if (fz) {
        push16(0x828B); NmiVramUpload_m1x1();
        push8(0x80); push16(0x828F); NmiFlushVramQueue_m1x1();
        lda8(rd8(DP(0x9A))); wr8(ABS(BG1HOFS), (u8)A);
        lda8(rd8(DP(0x90))); wr8(ABS(BG1HOFS), (u8)A);
        if (rd8(DP(0x94))) {
            lda8(rd8(DP(0x9E))); wr8(ABS(BG1VOFS), (u8)A);
            lda8(rd8(DP(0x9F))); wr8(ABS(BG1VOFS), (u8)A);
        } else {
            lda8(rd8(DP(0x9B))); wr8(ABS(BG1VOFS), (u8)A);
            lda8(rd8(DP(0x90)));
            and8(0x02);
            lda8(lsr8((u8)A));
            wr8(ABS(BG1VOFS), (u8)A);
        }
        for (int r = 0; r < 4; r++) {
            lda8(rd8(DP(0xA0 + 2 * r))); wr8(ABS(BG2HOFS + r), (u8)A);
            lda8(rd8(DP(0xA1 + 2 * r))); wr8(ABS(BG2HOFS + r), (u8)A);
        }
    }
    lda8((rd8(DP(0x95)) || rd8(DP(0x96))) ? 0xB1 : 0x81);
    wr8(ABS(NMITIMEN), (u8)A);
    wr8(ABS(SH_NMITIMEN), (u8)A);
    lda8(rd8(ABS(SH_INIDISP))); wr8(ABS(INIDISP), (u8)A);
    lda8(rd8(ABS(SH_TM))); wr8(ABS(TM), (u8)A);
    lda8(rd8(ABS(SH_TS))); wr8(ABS(TS), (u8)A);
    lda8(rd8(ABS(SH_HDMAEN))); wr8(ABS(HDMAEN), (u8)A);
    lda8(rd8(ABS(SH_TMW))); wr8(ABS(TMW), (u8)A);
    lda8(rd8(ABS(SH_TSW))); wr8(ABS(TSW), (u8)A);
    lda8(rd8(ABS(SH_CGWSEL))); wr8(ABS(CGWSEL), (u8)A);
    lda8(rd8(ABS(SH_CGADSUB))); wr8(ABS(CGADSUB), (u8)A);
    sep(0x30);
    nmi_sound_channel4(0);
    nmi_sound_channel4(1);
    nmi_sound_channel4(2);
    for (;;) {                              /* channel 3: BNE back to LDA $70 */
        lda8(rd8(DP(0x6D + 3)));
        if (fz) {
            X = rd8(DP(0x69 + 3)); nz8((u8)X);
            lda8(rd8(ABSX(SND_RING + 0xC0)));
            if (fz) break;
            wr8(ABSX(SND_RING + 0xC0), 0);
        } else {
            cmp8((u8)A, rd8(ABS(APUIO3)));
            if (!fz) continue;
            lda8(rd8(DP(0x69 + 3)));
            lda8(inc8((u8)A));
            and8(0x3F);
            wr8(DP(0x69 + 3), (u8)A);
            lda8(0x00);
        }
        wr8(DP(0x6D + 3), (u8)A);
        wr8(ABS(APUIO3), (u8)A);
        break;
    }
    wr8(ABS(NMI_FRAME_READY), inc8(rd8(ABS(NMI_FRAME_READY))));
    rep(0x30);
    DB = pull8(); nz8(DB);
    D = pull16(); nz16(D);
    Y = pull16(); nz16(Y);
    X = pull16(); nz16(X);
    A = pull16(); nz16(A);
    rti4();
}
void ResetVector_m1x1(void)
{
    ResetBootAll();
    for (;;) {
        do lda8(rd8(ABS(NMI_FRAME_READY))); while (fz);
        MainLoopFrame4();
    }
}
