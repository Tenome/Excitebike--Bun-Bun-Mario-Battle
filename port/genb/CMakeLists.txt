# Gen B engine target (Stadiums 3 and 4). It lives in its own directory so the
# framework's per-directory outputs (the generated ROM identity header, the
# rom.cfg cache and the staged mods/assets beside the executable) do not
# collide with the Gen A target's.
bunbun_game(BunbunS4SNESRecomp ${CMAKE_SOURCE_DIR}/src/gen-genb ${CMAKE_SOURCE_DIR}/recomp-genb
            ${CMAKE_SOURCE_DIR}/rom_identity_genb.txt "BunBun S3-4")
target_compile_definitions(BunbunS4SNESRecomp PRIVATE BUNBUN_GENB=1 BUNBUN_SAVE_PREFIX="saves_genb")

# Decompiled Gen B routines (c/hand/s4), bridged like Gen A's. The roster
# (per-character data) is the port's own: src/roster.c replaces
# c/hand/s4/roster_rom.c, which only the difftest links.
set(BUNBUN_C ${CMAKE_SOURCE_DIR}/../c)
set(_bunbun_hand_b ${BUNBUN_C}/runtime816.c ${BUNBUN_C}/hand/s4/riders.c ${BUNBUN_C}/hand/s4/vram.c ${BUNBUN_C}/hand/s4/race.c ${BUNBUN_C}/hand/s4/select.c ${BUNBUN_C}/hand/s4/misc.c ${BUNBUN_C}/hand/s4/collide.c ${BUNBUN_C}/hand/s4/from_s1.c ${BUNBUN_C}/hand/s4/leaves.c ${BUNBUN_C}/hand/s4/objects.c ${BUNBUN_C}/hand/s4/flow.c ${CMAKE_SOURCE_DIR}/src/roster.c)
target_sources(BunbunS4SNESRecomp PRIVATE ${_bunbun_hand_b} ${CMAKE_SOURCE_DIR}/src/r816_bridge.c)
set_source_files_properties(${_bunbun_hand_b} PROPERTIES
    COMPILE_DEFINITIONS R816_EXTERNAL_BUS=1
    INCLUDE_DIRECTORIES "${BUNBUN_C};${BUNBUN_C}/hand/s4")

# Native mode (src/native_driver.c): the whole game from the hand-written C,
# with the Reset handler (c/hand/s4/reset.c), the sound uploads through their
# own handshake (c/hand/s4/spc.c, renamed so the bridge's native-transfer
# versions stay the default) and the address -> routine table.
set(_bunbun_native_b ${BUNBUN_C}/hand/s4/reset.c ${BUNBUN_C}/hand/s4/spc.c)
target_sources(BunbunS4SNESRecomp PRIVATE ${_bunbun_native_b}
    ${CMAKE_SOURCE_DIR}/src/native_driver.c ${CMAKE_SOURCE_DIR}/src/hand_table_s4.c)
set_source_files_properties(${BUNBUN_C}/hand/s4/reset.c PROPERTIES
    COMPILE_DEFINITIONS R816_EXTERNAL_BUS=1
    INCLUDE_DIRECTORIES "${BUNBUN_C};${BUNBUN_C}/hand/s4")
set_source_files_properties(${BUNBUN_C}/hand/s4/spc.c PROPERTIES
    COMPILE_DEFINITIONS "R816_EXTERNAL_BUS=1;SpcUploadB_m1x1=SpcUploadB_hand_m1x1;SpcUploadFrom=SpcUploadFrom_hand"
    INCLUDE_DIRECTORIES "${BUNBUN_C};${BUNBUN_C}/hand/s4")
