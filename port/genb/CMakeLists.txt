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
set(_bunbun_hand_b ${BUNBUN_C}/runtime816.c ${BUNBUN_C}/hand/s4/riders.c ${BUNBUN_C}/hand/s4/vram.c ${BUNBUN_C}/hand/s4/race.c ${CMAKE_SOURCE_DIR}/src/roster.c)
target_sources(BunbunS4SNESRecomp PRIVATE ${_bunbun_hand_b} ${CMAKE_SOURCE_DIR}/src/r816_bridge.c)
set_source_files_properties(${_bunbun_hand_b} PROPERTIES
    COMPILE_DEFINITIONS R816_EXTERNAL_BUS=1
    INCLUDE_DIRECTORIES "${BUNBUN_C};${BUNBUN_C}/hand/s4")
