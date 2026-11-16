# Gen A and Gen B, routine by routine

Made by `tools/abdiff.py --a s1 --b s4` from the hand-written C, with the RAM
named on both sides and ROM addresses masked. Pairs come from the
cross-version function maps (`build/xm`).

| | Routines |
|---|---|
| same | 130 |
| changed | 203 |
| only in S1 | 74 |
| only in S4 | 199 |

## Changed (most similar first)

| S1 | S4 | routine | similarity | lines differing |
|---|---|---|---|---|
| `80C09F` | `80BDDC` | PlaceRival | 99% | 1 |
| `80973F` | `8097B9` | BroadcastTotals | 98% | 2 |
| `819C98` | `819D71` | VelocityDirection | 98% | 2 |
| `80B702` | `828E8E` | SaveHeaderCheck | 98% | 2 |
| `80ABD7` | `80AC0E` | RiderThrottle | 98% | 5 |
| `80C503` | `80C290` | RaceStartLights | 97% | 2 |
| `8193E2` | `8194B6` | SpawnExhaust | 97% | 2 |
| `80BE6A` | `80BB82` | WalkToStartLane | 97% | 2 |
| `8099E7` | `8098DE` | EndScreenTimes | 97% | 2 |
| `80E319` | `80F2D2` | Mode0C | 97% | 2 |
| `80AEC6` | `80AF3F` | CpuRidersStep | 97% | 2 |
| `8194BE` | `819592` | DrawDustPair | 97% | 2 |
| `819EF5` | `819FCE` | ObjectFacing | 97% | 2 |
| `80ABA0` | `80ABCD` | ThrottleFrom | 96% | 2 |
| `8196B4` | `819788` | SpawnObject | 95% | 4 |
| `8097FF` | `828F6F` | SaveBestRecords | 95% | 6 |
| `80C04B` | `80BD88` | UpdateRiderDistances | 95% | 4 |
| `80EB04` | `80FB58` | RiderBumpCrash | 95% | 2 |
| `81F8BB` | `81F8BB` | BsxPollStream | 95% | 6 |
| `80B158` | `80B208` | RaceBookkeeping | 94% | 2 |
| `80DE23` | `80DD67` | BonusResultStart | 94% | 2 |
| `80CB47` | `80C997` | BallHit | 94% | 4 |
| `819670` | `819744` | MoveObjectX | 94% | 4 |
| `80C654` | `80C3E1` | RaceTimer | 94% | 7 |
| `8098B2` | `828259` | QueueTimeDigits | 94% | 6 |
| `81B731` | `81B9DB` | ReleaseBalloons | 94% | 6 |
| `80B015` | `80B0D0` | RetargetRival | 94% | 2 |
| `809F68` | `809E9F` | Mode12_AfterEnd | 94% | 10 |
| `809AF9` | `8099F0` | EndScreenCode | 94% | 4 |
| `809DE0` | `809CD1` | EndScreenStart | 93% | 2 |
| `80B02F` | `80B0EA` | CpuAvoidHurdle | 93% | 10 |
| `80ED42` | `80FDDE` | CrossFinish | 93% | 4 |
| `809942` | `8283A1` | QueueFiveDigits | 93% | 4 |
| `80AB2A` | `80AB46` | BallOrGauge | 93% | 2 |
| `819DC6` | `819E9F` | DrawObjectSprites | 92% | 10 |
| `80C5FE` | `80C38B` | FlagRidersAtLine | 92% | 4 |
| `80C475` | `80C1F3` | CrashRecovery | 92% | 2 |
| `80DE07` | `80DD45` | Mode0A | 92% | 2 |
| `80EB39` | `80FBAE` | SlowPieceBrake | 91% | 6 |
| `81982B` | `8198FF` | FadeInObject | 91% | 4 |
| `80FFA9` | `80FFA9` | IrqVector | 91% | 8 |
| `80DB89` | `80DA77` | BuildCourseColumn | 91% | 20 |
| `80ECE4` | `80FD7E` | DrawSparksAndMarkers | 90% | 10 |
| `80C604` | `80C391` | SpeedGauge | 90% | 9 |
| `809A87` | `80997E` | EndScreenRecord | 90% | 8 |
| `80BFD0` | `80BD0D` | ScrollByPlayerSpeed | 90% | 3 |
| `809EAB` | `809DDA` | StartCredits | 89% | 8 |
| `8084A8` | `828180` | ForceBlank | 89% | 2 |
| `818A4D` | `818A4D` | LoadRaceColumn | 88% | 39 |
| `819778` | `81984C` | FollowObjectPath | 88% | 9 |
| `819A8D` | `819B5D` | QueueMapTile | 88% | 6 |
| `808A6D` | `80862A` | Mode0D_Setup | 88% | 5 |
| `80E3DF` | `80F3AD` | PlaceLastRival | 88% | 2 |
| `80FFA5` | `80FFA5` | ResetVector | 88% | 2 |
| `80E983` | `80F9D0` | RiderDropStep | 87% | 5 |
| `80C34C` | `80C0B4` | RiderLandCheck | 87% | 4 |
| `80C7EB` | `80C58D` | TickRaceClock | 86% | 5 |
| `80D31F` | `80D145` | MarkerObjectStep | 86% | 5 |
| `80DDA7` | `80DC95` | PlayerStop | 86% | 2 |
| `80E3D4` | `80F3A2` | DemoExitBoost | 86% | 2 |
| `819852` | `819926` | ResultsBonusCount | 86% | 24 |
| `80A9BA` | `80A9C1` | Mode03_Race | 85% | 19 |
| `8084B7` | `82818F` | ClearTilemaps | 85% | 9 |
| `80C225` | `80BF5C` | RiderLanding | 85% | 25 |
| `80A89C` | `80A859` | RacePlaceRiders | 85% | 31 |
| `80A03F` | `809F76` | GrowBar087E | 85% | 6 |
| `80A4FE` | `80A457` | FinalSchedule | 85% | 9 |
| `80AFFB` | `80B0B6` | CpuSteer | 85% | 4 |
| `80EBDF` | `80FC5F` | UpdateLanes | 85% | 40 |
| `80E9F3` | `80FA40` | UseArrowPiece | 84% | 10 |
| `80E8D5` | `80F8F7` | LanePieceTablesLong | 84% | 3 |
| `80C41D` | `80C19B` | FinishStep | 84% | 12 |
| `80C37B` | `80C0E3` | CrashLand | 83% | 2 |
| `80E3F2` | `80F3BE` | DemoExitSkip35 | 83% | 2 |
| `80E8B4` | `80F8D6` | RiderPieceAt | 83% | 2 |
| `80CDF6` | `80CC18` | WheelSpark | 83% | 18 |
| `8197E2` | `8198B6` | SkyFadeObject | 83% | 7 |
| `80EDEF` | `80FE93` | PaceBonus | 83% | 15 |
| `80BF88` | `80BCBD` | RaceMoveStep | 82% | 10 |
| `80CD54` | `80CB78` | RiderPose | 82% | 21 |
| `80EAD1` | `80FB25` | PlayerOverheatWarn | 81% | 8 |
| `809C0D` | `809B0D` | EntryNumberEditor | 81% | 33 |
| `80DA99` | `80D97C` | PlayerTileState | 81% | 22 |
| `80AF91` | `80B04E` | PieceOK | 80% | 2 |
| `80C52E` | `80C2BB` | DrawStartFlags | 80% | 2 |
| `80E3EE` | `80F3BA` | DemoExitSkip40 | 80% | 2 |
| `80E4FC` | `80F4C8` | DemoPushStart | 80% | 13 |
| `80E773` | `80F755` | AnimateAllWheels | 80% | 2 |
| `80EDB2` | `80F15B` | NextColumnLong | 80% | 2 |
| `80EDC0` | `80E994` | AddScoreLong | 80% | 2 |
| `80EE58` | `80E938` | QueueScoreDigitsLong | 80% | 2 |
| `819F1E` | `819FF7` | ApplyVelocityX | 80% | 2 |
| `819F4C` | `81A025` | ApplyVelocity | 80% | 2 |
| `80FFA1` | `80FFA1` | NmiVector | 80% | 31 |
| `80E9C1` | `80FA0E` | FlattenPlayerPiece | 79% | 9 |
| `80D97E` | `80D81B` | Mode09 | 79% | 38 |
| `80DB44` | `80DA2A` | BonusRunOut | 78% | 9 |
| `808D0C` | `8089B4` | StartModeWithSound | 78% | 4 |
| `80BEB7` | `80BBCF` | WalkBackOn | 78% | 27 |
| `8195A3` | `819677` | SparkPieceStep | 77% | 14 |
| `80DEB3` | `80DDEF` | BonusCountUp | 77% | 16 |
| `80AFF2` | `80B0AD` | CpuReleaseDpad | 77% | 3 |
| `80C3B3` | `80C131` | StartJump | 77% | 14 |
| `80DCEA` | `80DBD8` | CourseRowTiles | 76% | 8 |
| `80A0AA` | `809FE1` | BsxCheckSerial | 76% | 5 |
| `80EBB5` | `80FC35` | LandingStep | 76% | 7 |
| `80C12C` | `80BE66` | RivalTarget | 75% | 23 |
| `809D07` | `809C14` | EndScreenPrompt | 75% | 12 |
| `80E7AC` | `80F78E` | RunRiderPieces | 75% | 68 |
| `80EB8E` | `80FC03` | SlowPieceStep | 75% | 13 |
| `80B498` | `8286EB` | TickTimers | 74% | 9 |
| `80EA6A` | `80FAB7` | CollectPickupPiece | 74% | 16 |
| `80CF0D` | `80CD34` | TemperatureGauge | 74% | 53 |
| `808EA0` | `808B26` | SelectTimerDigits | 74% | 15 |
| `809236` | `808FF1` | RaceHudInit | 74% | 5 |
| `80C1C9` | `80BF03` | LaunchBall | 73% | 12 |
| `80EAFF` | `80FB53` | SetJumpState1 | 73% | 3 |
| `80EE68` | `80FF0C` | QueueScoreDigits | 72% | 13 |
| `80E043` | `80DF55` | AnimSourceFromFrame | 72% | 7 |
| `80EBC1` | `80FC41` | ResetAndLand | 71% | 4 |
| `809B4C` | `809A43` | EndScreenChoice | 71% | 21 |
| `80A6B2` | `80A66F` | EraseCollectedCoin | 70% | 31 |
| `80CCE1` | `80CAE5` | DrawRider | 70% | 31 |
| `8199CB` | `819A9B` | FadePaletteStep | 69% | 17 |
| `80AFD5` | `80B090` | CpuThrottle | 69% | 10 |
| `81A0B5` | `81A1C8` | EngineSound | 68% | 36 |
| `81A620` | `81A747` | SetHdmaPreset | 67% | 43 |
| `80BC55` | `80B912` | ScaleIrqLine | 67% | 6 |
| `80E939` | `80F95B` | RiderHighJump | 67% | 5 |
| `80E958` | `80F9A5` | RiderJumpBase | 67% | 10 |
| `80AE33` | `80AE97` | BrakeRider | 66% | 21 |
| `808B14` | `8086F8` | Mode0E_Setup | 65% | 16 |
| `809E46` | `809D37` | BeginFadeInBg3 | 65% | 12 |
| `80DB6F` | `80DA55` | BonusTimerStep | 65% | 12 |
| `80AF9F` | `80B059` | RiderAhead | 64% | 24 |
| `808369` | `82805F` | InitPpuRegisters | 64% | 39 |
| `80D038` | `80CE5D` | ClearMidScreenRiders | 62% | 22 |
| `80D813` | `80E5B0` | SetupJumpContest | 62% | 38 |
| `80E930` | `80F952` | RiderHalfDrop | 62% | 5 |
| `80C9C4` | `80C776` | RiderCollisions | 61% | 114 |
| `80D29F` | `80D12B` | MarkerObjects | 60% | 21 |
| `81BBE4` | `81BE8E` | IdleCycleObject | 59% | 22 |
| `80E91E` | `80F940` | RiderKickUp | 59% | 7 |
| `808EF7` | `808B75` | TurnSelectWheels | 58% | 27 |
| `809BC8` | `809ABF` | EndScreenScrollIn | 58% | 20 |
| `80AF07` | `80AF87` | CpuLaneDecision | 56% | 38 |
| `80CC3D` | `80CA98` | DrawAllRiders | 55% | 43 |
| `80E947` | `80F969` | RiderPairHeight | 55% | 5 |
| `80A7AF` | `80A76C` | RedrawUsedItem | 54% | 43 |
| `808A9D` | `80865A` | Mode0D_IntroText | 50% | 56 |
| `81B896` | `81BB40` | RivalSequence_B896 | 49% | 25 |
| `80A10E` | `80A070` | PackEntryCode | 47% | 31 |
| `80A68F` | `80A650` | DrawHudLabelB | 47% | 9 |
| `80B873` | `829182` | LoadScene | 47% | 85 |
| `80A66D` | `80A62E` | DrawHudLabelA | 46% | 7 |
| `80D20F` | `80D08A` | CoinCollected | 46% | 62 |
| `80C8E9` | `80C68D` | ShowCourseBest | 43% | 13 |
| `80916B` | `808F26` | PlayerSelectStart | 43% | 34 |
| `809248` | `8091FC` | RaceSetup | 43% | 32 |
| `81BE4C` | `81C0F6` | PodiumWalkIn | 43% | 51 |
| `80E377` | `80F330` | DemoInputStep | 43% | 43 |
| `81A126` | `81A24B` | SelectWheelPosition | 41% | 44 |
| `81B836` | `81BAE0` | RivalSequence_B836 | 40% | 21 |
| `808C2F` | `80889B` | Mode02_SegmentCard | 37% | 17 |
| `809982` | `809879` | BsxReadSerial3 | 36% | 14 |
| `80AB3D` | `80AB59` | SegmentTimer | 36% | 54 |
| `809206` | `808FA6` | ClearRaceState | 35% | 40 |
| `80876B` | `8083D2` | NmiFixedUploads | 35% | 98 |
| `80D3C1` | `80D1E6` | DrawSmallObject | 34% | 31 |
| `81B9D5` | `81BC7F` | RivalSequence_B9D5 | 33% | 28 |
| `80E2B6` | `80F266` | StartDemo | 32% | 25 |
| `808C53` | `8088BF` | Mode02_Setup | 32% | 96 |
| `80D0BA` | `80CF32` | UpdatePlayerPlace | 31% | 31 |
| `809053` | `808DBA` | SpriteTextRow | 31% | 40 |
| `81BAAD` | `81BD57` | RivalSequence_BAAD | 30% | 23 |
| `81BC4B` | `81BEF5` | RivalSequence_BC4B | 29% | 24 |
| `80819F` | `8081A9` | DrawClock | 27% | 59 |
| `8085A6` | `80824E` | PackOamHighTable | 26% | 23 |
| `80ADF9` | `80AE33` | AccelerateRider | 26% | 52 |
| `8089ED` | `8085A9` | RunGameModeScript | 24% | 77 |
| `80B2A9` | `80B2DE` | VramQueueTiles | 16% | 32 |
| `81A01B` | `81A0F4` | LoadCourse | 15% | 141 |
| `80DF01` | `80DE5E` | BonusResultStep | 13% | 83 |
| `80B3FE` | `80B435` | VramQueueCopy | 12% | 100 |
| `808F2D` | `808BAE` | DrawSelectWheels | 11% | 78 |
| `8082B6` | `82862C` | QueueSound | 0% | 16 |
| `8092A0` | `81C641` | RaceResultsSetup | 0% | 156 |
| `80AA78` | `80AA94` | RaceBannerSprites | 0% | 2 |
| `80AF4C` | `80AFE9` | CpuSteerUp | 0% | 2 |
| `80AF71` | `80B01E` | CpuSteerDown | 0% | 2 |
| `80BC61` | `80B98E` | Mul16 | 0% | 7 |
| `80C8EE` | `80C692` | RecordCourseTime | 0% | 48 |
| `80DA31` | `80D914` | BonusBannerSprites | 0% | 2 |
| `80DB28` | `80DA0E` | ScrollFarLayer | 0% | 17 |
| `80DC8D` | `80DB7B` | CourseColumn | 0% | 31 |
| `80DD0E` | `80DBFC` | PlayerCrashStep | 0% | 57 |
| `80DD82` | `80DC70` | PlayerColumnCheck | 0% | 45 |
| `80E929` | `80F94B` | SetJumpBase | 0% | 7 |
| `80E975` | `80F9C2` | SetRiderPair | 0% | 9 |
| `819520` | `8195F4` | ObjectStepDispatch | 0% | 10 |
| `81A1E5` | `81A304` | Random | 0% | 16 |
| `81B976` | `81BC20` | RivalCycle_B976 | 0% | 2 |
| `81BCC8` | `81BF72` | RivalCycle_BCC8 | 0% | 2 |

## Only in S1

* `80823C` BroadcastEventInject
* `808322` FadeStep
* `808539` DmaKick
* `808541` DmaCopyToWram
* `808589` InitFadeState
* `808D25` Mode01
* `808D2F` RiderSelectFirst
* `808DE6` RiderSelectSecond
* `80915A` SegmentNext
* `809201` EnableHudSplit
* `8095AE` DrawText95C4
* `809698` Mode04_FreeRunSchedule
* `809D43` DrawEntryScreen
* `809E1D` EndScreenTimeout
* `809E6E` Mode10_Ending
* `80A0BC` ShowEntryPassword
* `80A1BC` PackRecordTime
* `80A231` PackEntryNumber
* `80A2B0` LoadSavedEntry
* `80A34B` EepromReadWords
* `80A387` EepromSendCommand
* `80A3AC` DigitsToNumber
* `80A458` SumBytes0850
* `80A5AC` DrawRacerNameLow
* `80A5B7` DrawRacerName
* `80AF94` PieceFlatish
* `80B226` ClearOamBuffer
* `80B538` RiderPaletteWords
* `80BCCD` Mul16c
* `80BF43` RiderStartCheck
* `80BFBA` AddSpeedFraction
* `80C187` ClearRiderState
* `80C805` ShowRaceClock
* `80C877` ShowBestTime
* `80D069` ClipRiderSprite
* `80D113` DrawPlace
* `80D14B` DrawLap
* `80D38E` DrawCoinIcons
* `80D3F7` ResetCoinMarkers
* `80D7B2` SetBonusRow
* `80D8FB` StartBonusLabel
* `80D906` ShowBonusLabel
* `80DDF9` StartDemoHdma
* `80E004` CheckEndingMinute
* `80E059` ShowBonusBest
* `80E775` AnimateWheels
* `80E9A8` ClearJumpState
* `80E9B1` StartJumpState
* `80EB0C` SpeedBumpCrash
* `8197CC` RivalCountdown_97CC
* `81B64A` RivalTumble_B64A
* `81B67F` RivalSequence_B67F
* `81B7B9` RivalWaitRandom_B7B9
* `81B7E0` RivalAlternate_B7E0
* `81B819` RivalWaitStep_B819
* `81B879` RivalWaitStep_B879
* `81B8ED` RivalWaitRandom_B8ED
* `81B914` RivalAlternate_B914
* `81B94D` RivalPause_B94D
* `81B9B6` RivalWaitStep_B9B6
* `81BA2E` RivalWaitRandom_BA2E
* `81BA55` RivalAlternate_BA55
* `81BA8E` RivalWaitStep_BA8E
* `81BAE8` RivalWaitStep_BAE8
* `81BB05` RivalSequence_BB05
* `81BB49` RivalWaitRandom_BB49
* `81BB70` RivalAlternate_BB70
* `81BBA9` RivalBlink_BBA9
* `81BC2E` RivalWaitStep_BC2E
* `81BCA1` RivalWaitRandom_BCA1
* `81BD06` RivalWaitStep_BD06
* `81BD23` RivalSequence_BD23
* `81BD67` ClearObjectPalette
* `81BECE` RivalWalkAway_BECE

## Only in S4

* `8089D7` SelectMain
* `808D28` SelectHideRest
* `808D38` SelectSlideY
* `808D6C` SelectTransition
* `808F10` ChangeModeLong
* `808F15` NextMode
* `808F17` ChangeMode
* `808F9D` SetupHdmaFlag
* `809004` ShowCoinRank
* `8091F4` StartRaceBoard
* `80924C` RaceResultsSetup
* `809471` ShowItemPoints
* `80954E` AddBcdDigits
* `809567` ShowDigitDifference
* `8095A2` BroadcastEndScreens
* `809608` ResultsPlacePoints
* `8096E0` ResultsJumpPoints
* `809C58` SetupJumpScreen
* `809C79` EntryScreenFill
* `809D0E` LoadEntryScreenStep
* `809D74` ModeCountdown
* `809FF3` ShowEntryCode
* `80A129` ReadBroadcastDate
* `80A17E` StoreEntryNumber
* `80A39C` CountCourseBits
* `80A4DA` DrawRaceHud
* `80AC04` RiderThrottleLong
* `80B051` IsPieceFlat
* `80B387` QueueTileBlockLong
* `80B38B` QueueTileBlock
* `80B431` QueueTileRowsLong
* `80B5C3` StartBonusMode
* `80B5F8` Mode09_BonusIntro
* `80B65B` StartBonusResult
* `80B736` EndJumpMode
* `80B76A` StartJumpMode
* `80B7A0` Mode17_BonusIntro
* `80B7EC` CourseSelectStep
* `80B7F1` JumpRunStepsMode
* `80B91E` ParallaxLines
* `80B9FA` Mul16b
* `80BC5B` RiderAtStart
* `80BCEF` AddSpeedToXLong
* `80BCF3` AddSpeedToX
* `80BD09` ScrollByPlayerSpeedLong
* `80C5B0` QueueRaceTime
* `80C619` QueueBestTimeLong
* `80C623` QueueBestTime
* `80CA89` DrawAllRidersLong
* `80CD0A` DrawRiderShadowPairLong
* `80CE8C` PlaceRiderShadowLong
* `80CE96` PlaceRiderShadow
* `80CF8B` QueuePlaceBadgeHud
* `80CF9D` QueuePlaceBadge
* `80D21C` ResetCoinCounts
* `80D5D9` SetupBonusScreen
* `80D656` StartBonusCourse
* `80D79D` StartBonusCount
* `80D7AB` QueueBonusCount
* `80D8EF` CheckBonusTime
* `80DCE6` AddBonusPoints
* `80DD2B` StartBonusTry
* `80DDAE` QueueResultsCount
* `80DE3D` QueueNameplate
* `80DF31` ResultsWait
* `80DFB9` EndAtBonusMinute
* `80E4A9` SetupCourseScreen
* `80E4CB` ShowJumpResult
* `80E6AB` FillBonusColumns
* `80E6D2` JumpContestMode
* `80E74D` CheckCourseTime
* `80E771` DrawJumpRank
* `80E7C1` DrawJumpRiderSprites
* `80E835` SetJumpRiderSprites
* `80E85B` JumpContestFrame
* `80E8E5` JumpResultTally
* `80E933` JumpDistanceStepNear
* `80E93C` JumpResultCount
* `80E956` ShowJumpCount
* `80E998` JumpResultBonus
* `80E9B2` ShowBonusPoints
* `80E9FC` JumpTotalStep
* `80EA01` JumpResultBoardStep
* `80EA3E` JumpResultWait
* `80EA68` JumpResultBoard
* `80EB83` DrawCountdownLights
* `80EBBC` DispatchBy85B
* `80EBD0` ReadBonusTrack
* `80ECD8` ScrollBySpeed16B
* `80ECF4` JumpRunUp
* `80ED1D` BuildBonusColumn
* `80EE40` CourseColumnB
* `80EE8B` BlankQueueRowB
* `80EE9D` CopyPieceColumn
* `80EEB0` BonusRowList
* `80EEBB` PlayerCrashStepB
* `80EF37` PlayerColumnCheckB
* `80EF5C` PlayerStopB
* `80EF64` PlayerHaltB
* `80F02D` DemoStepDispatch
* `80F03B` ScrollDelta
* `80F048` StopWhenStill
* `80F07E` ShowLastDistance
* `80F097` BonusSignStep
* `80F107` JumpHudNumbers
* `80F15F` BuildScoreDigits
* `80F1D6` SetJumpScroll
* `80F757` SpinWheels
* `80F97A` PlayerSpeedBoost
* `80F9F5` ClearJumpFlags
* `80F9FE` SetJumpKind
* `80FB60` BoostOnTile
* `80FE56` NextColumnLong
* `80FE64` AddScoreLong
* `80FEFC` QueueBonusDigitsLong
* `8198A0` StepAnimTimer
* `81B8F4` FallingObjectStep
* `81B929` StepObjectScript
* `81BA63` ObjectAdvanceRandom20
* `81BA8A` ObjectBlinkA
* `81BAC3` ObjectAdvance8
* `81BB23` ObjectAdvance20
* `81BB97` ObjectAdvanceRandom10
* `81BBBE` ObjectBlinkB
* `81BBF7` ObjectAdvanceRandom16
* `81BC60` ObjectAdvanceOr4
* `81BCD8` ObjectAdvanceRandom0A
* `81BCFF` ObjectBlinkC
* `81BD38` ObjectAdvanceOr5
* `81BD92` ObjectAdvance80
* `81BDAF` ObjectCycleA
* `81BDF3` ObjectAdvanceRandom09
* `81BE1A` ObjectBlinkD
* `81BE53` ObjectFlicker
* `81BED8` ObjectAdvance9
* `81BF4B` ObjectAdvanceRandom10B
* `81BFB0` ObjectAdvanceC0
* `81BFCD` ObjectCycleB
* `81C011` ClearObjectFrame
* `81C178` ObjectWalkCycle
* `81C1AE` CourseSelectPreview
* `81C29A` JumpRunSteps
* `81C2D9` JumpRunStart
* `81C331` JumpRun
* `81C393` JumpPitchStep
* `81C3A5` CourseMinuteCheck
* `81C3B1` LeaveJumpContest
* `81C3BA` DrawJumpCrowd
* `81C483` JumpRiderStep
* `81C56E` ParkJumpRider
* `81C579` SpinWheels8
* `81C5AA` MoveAndScroll
* `81C5B5` ScrollJumpRider
* `81C5D7` SlotRiderShadow
* `81C608` DispatchByDistance
* `81C62B` PitchUp4
* `81C636` PitchUp8
* `81C643` PitchDown8
* `81C64E` FadeApuVolume
* `81C6B4` QueueBlinkBanner
* `81C6F9` CoinMilestone
* `81C724` StepJumpBanner
* `81C743` SetJumpBanner
* `81C75E` CheckLaneTile
* `81C7C3` ClearOnBonusTiles
* `81C7E0` JumpDistanceStep
* `81C7EF` JumpResultDistance
* `81C84F` JumpResultTotal
* `81C883` QueueResultsTiles
* `81C8E1` QueueHalfUnits
* `81C956` JumpResultSkip
* `81C9A7` BuildJumpScore
* `81CA05` Hundreds
* `81CA20` ScaleJumpCount
* `81CA37` QueueStadiumBadge
* `82811D` SetupHdmaChannels
* `8282EA` QueueFontTime
* `8283E2` QueueDigitsAt
* `828424` QueueFontDigits
* `82846B` QueueEntryDigits
* `8284B2` ByteTable84BA
* `8284C5` ResetSoundState
* `8284EE` QueuePlayerMarkers
* `828525` PackSpriteSizes
* `828698` StepFade
* `828701` PlaceMinuteMarker
* `828782` HideAllSprites
* `8287FD` RiderSpriteSets
* `828965` RiderGfxCycle
* `82899B` LoadRiderPalettes
* `828BB4` LoadStadiumPalette
* `828BD8` UnpackToLowRam
* `828BFE` UnpackToWram
* `828CC5` ReadStreamByte
* `828CD4` ReadStreamWord
* `828CF0` SpeedReadout
* `828DC7` BestTimeBlink
* `829028` StadiumBonusBanner
* `829159` AddStadiumBonus

## Same

DmaFillZero, SpcUpload, NmiFlushVramQueue, Mode00_IntroWait, ModeStepNext, Mode0E_TitleWait, FinishModeSetup, StartSession, BsxReadSerialByte, Mode0F_EndScreen, EndScreenScore, BeginFadeIn, Mode11, StepColourCycle, Add32, MergeBsxFlags, SendSpeedToApu, ThrottleAll, RiderOnRamp, BrakeOnTurn, CpuReleaseButtons, LaneFromY, ReadRiderPieces, DrawBigSprite, ScreenOff, VramQueueTilesLong, VramQueueTiles20, StepLfsr, ReadJoypads, ModeNop, SaveChecksum, Nop_BD39, RiderStepAll, EmptyBE2D, RiderStateStep, RiderRunOut, RiderStartLane, RiderShadowY, ScrollRivalsRelative, SpawnRivals, GroundHeightAbove, HeightAbove, SetRiderLean, Tick08CC, RiderHeightStep, RaceClockStep, DivideAX, RiderLaneBand, Reach20, Reach2, SortAndDrawRiders, DrawRiderShadowPair, SortRidersByPosition, ComputePlaces, MirrorJumpRider, StepObjectX8, StepObjectX, ClearWords1530, SetupSpriteRow15, DispatchBy85, BlankQueueRow, CourseRowList, PlayerHalt, EmptyDDF8, ResetDemoCourse, SetRaceBgMode, TemperatureGaugeJmp, StartMode01, DemoPadDisplay, RunAllRiderPieces, PieceAction, RiderLanePieceAt, LanePieceAt, LanePieceTables, MarkFastRider, RiderGroundHeights, RiderGroundHeightsFrom, HeightBelow, StartJumpBase30, JumpBase30, JumpBase10, RunPieceActions, ClearRiderAction, ItemFlashState, NextColumn, AddScore, QueueScoreDigitsHud, EmptyVector, RaceRowList, FillLaneColumn, BlankQueueRowPal2, CopyTrackColumn, SpawnPlayerSpark, SpawnDustPuff, StepSpriteSlots, EmptyStep9554, StepFallingSprite, ConfettiStep, StepSpark, DrawSpark, MoveObjectX8, UpdateObjects, EmptyStep9828, FindObjectSlot, RivalDispatch_9761, Empty9956, FadeSkyColours, DrawBigBall, SteerObjectStep, AimVelocity, AimRatio, SinCos, EmptyHook, RivalDispatch_B639, EmptyB67E, RivalDispatch_B6B3, DriftUpObject, RivalDispatch_B7A2, RivalDispatch_B8D6, RivalDispatch_BA17, RivalDispatch_BB32, RivalDispatch_BC8A, RivalDispatch_BD52, FrameSequenceA, FrameSequenceB, BsxOpenTimeChannel, ResetMinuteFrame, BsxFindFile, BsxDetect, BsxProbeReceiver
