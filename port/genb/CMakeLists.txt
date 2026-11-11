# Gen B engine target (Stadiums 3 and 4). It lives in its own directory so the
# framework's per-directory outputs (the generated ROM identity header, the
# rom.cfg cache and the staged mods/assets beside the executable) do not
# collide with the Gen A target's.
bunbun_game(BunbunS4SNESRecomp ${CMAKE_SOURCE_DIR}/src/gen-genb ${CMAKE_SOURCE_DIR}/recomp-genb
            ${CMAKE_SOURCE_DIR}/rom_identity_genb.txt "BunBun S3-4")
target_compile_definitions(BunbunS4SNESRecomp PRIVATE BUNBUN_GENB=1 BUNBUN_SAVE_PREFIX="saves_genb")
