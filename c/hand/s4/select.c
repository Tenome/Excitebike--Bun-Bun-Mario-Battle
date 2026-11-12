/*
 * Stadium 3-4 (Gen B): the PLAYER SELECT carousel, hand-written.
 *
 * The portraits sit on an ellipse and turn as one ring: portrait i is at
 * angle a0 + ring offset (a0 = $1554, the first portrait). Which portrait is
 * in front, where each is drawn and what it shows come from the roster
 * (roster.h), so the port can offer more characters than the original five.
 * The original keeps one angle per portrait in $1554-$1558 and their screen
 * positions in $155A-$1563; those are kept for portraits 0-4, portraits
 * beyond them are computed from a0.
 */
#include "hand/hand.h"
#include "roster.h"

void VramQueueTiles20_m1x1(void);

/* $82:862C QueueSoundB (RTL; entered with M=0 or M=1): append sound command
 * A to the ring at $1400 (index $65, 64 entries); $71 = last command. */
static void queue_sound_body(void)
{
    push8(getP());
    rep(0x10);
    push16(X);
    sep(0x30);
    wr8(ABS(0x0071), (u8)A);
    X = rd8(ABS(0x0065));
    wr8(ABS(0x1400 + X), (u8)A);
    X = (u8)(X + 1);
    SET_A8((u8)X);
    and8(0x3F);
    wr8(ABS(0x0065), (u8)A);
    rep(0x10);
    X = pull16();
    setP(pull8());
    retl();
}
void QueueSoundB_m1x1(void) { queue_sound_body(); }
void QueueSoundB_m0x1(void) { queue_sound_body(); }

/* $81:A304 RandomB (RTL), as Stadium 1's: mixes the latched H/V counters,
 * the frame counter $63/$64 and the broadcast clock into A. */
static void random_body(void)
{
    lda8(rd8(ABS(0x2137)));
    lda8(rd8(ABS(0x213F)));
    adc8(rd8(ABS(0x213C)));
    adc8(rd8(ABS(0x213C)));
    adc8(rd8(ABS(0x213D)));
    adc8(rd8(ABS(0x213D)));
    eor8(rd8(DP(0x63)));
    adc8(rd8(DP(0x64)));
    adc8(rd8(LONG(0x7FFFFF)));
    adc8(rd8(LONG(0x7FFFFA)));
    adc8(rd8(LONG(0x7FFFFB)));
    retl();
}
void RandomB_m1x1(void) { random_body(); }
void RandomB_m1x0(void) { random_body(); }

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
    wr8(ABS(0x4202), (u8)A);
    lda8(rd8(LONG(0x81A427 + X)));
    wr8(ABS(0x4203), (u8)A);
    lda8(rd8(LONG(0x81A527 + X)));
    wr8(DP(0x02), (u8)A);
    wr8(DP(0x03), 0);
    lda8(rd8(ABS(0x4216)));
    lda8(asl8((u8)A));
    lda8(rd8(ABS(0x4217)));
    adc8(0x00);
    wr8(DP(0x00), (u8)A);
    wr8(DP(0x01), 0);
    lda8(rd8(LONG(0x81A327 + X)));
    wr8(ABS(0x4203), (u8)A);
    lda8(rd8(LONG(0x81A627 + X)));
    wr8(DP(0x06), (u8)A);
    wr8(DP(0x07), 0);
    lda8(rd8(ABS(0x4216)));
    lda8(asl8((u8)A));
    lda8(rd8(ABS(0x4217)));
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
            lda8(rd8(ABS(0x1559)));
            if (!fz) {
                lda8(rd8(DP(0x20)));
                and8(0x03);
                cmp8((u8)A, rd8(ABS(0x198C)));
                if (!fz) {
                    lda8(0x41);
                    push8(0x81); push16(0xA297); QueueSoundB_m1x1();
                    wr8(ABS(0x1559), 0);
                }
            }
        }
    }
    retl();
}

/*
 * $80:8B75 SelectRotate (RTL): start turning when left or right is pressed
 * ($1559 = $80:8BAA[pad]: +2 / -2, $198C = the direction); while turning,
 * a tick sound every 4 frames; turn every portrait and place it.
 */
void SelectRotate_m1x1(void)
{
    lda8(rd8(ABS(0x1559)));
    int tick = 0;
    if (!fz) {
        tick = 1;
    } else {
        lda8(rd8(DP(0x20)));
        and8(0x03);
        if (!fz) {
            wr8(ABS(0x198C), (u8)A);
            X = (u8)A; nz8((u8)X);
            lda8(rd8(ABS(0x8BAA + X)));
            wr8(ABS(0x1559), (u8)A);
            tick = 1;
        }
    }
    if (tick) {
        lda8(rd8(DP(0x63)));
        and8(0x03);
        if (fz) {
            lda8(0x40);
            push8(0x80); push16(0x8B95); QueueSoundB_m1x1();
        }
    }
    ldx8(0x04);
    do {
        lda8(rd8(ABS(0x1554 + X)));
        fc = 0;
        adc8(rd8(ABS(0x1559)));
        wr8(ABS(0x1554 + X), (u8)A);
        push8(0x80); push16(0x8BA5); CarouselPos_m1x1();
        X = dec8((u8)X);
    } while (!fn);
    retl();
}

/*
 * $80:8DBA SelectName (RTL), portrait X in front: its name as six 8x8
 * sprites at $1700 (x $68-$90, y $84): tiles row * 16 + 6.. (row from the
 * roster: the portrait, or the Stadium's own for the shared slot),
 * attribute $21, small.
 */
void SelectName_m1x1(void)
{
    lda8(0x68);
    for (int k = 0; k < 6; k++) {
        wr8(ABS(0x1700 + k * 4), (u8)A);
        if (k < 5) { fc = 0; adc8(0x08); }
    }
    lda8(0x84);
    for (int k = 0; k < 6; k++)
        wr8(ABS(0x1701 + k * 4), 0x84);
    lda8((u8)X);
    cmp8((u8)X, 0x03);
    int st = rd8(LONG(0x81800C)) != 0;
    u8 row = roster_select_name_row(st, X);
    if (fz) {
        push8((u8)X);
        lda8(rd8(LONG(0x81800C)));
        X = (u8)A;
        lda8(rd8(LONG(0x808E46 + X)));
        X = pull8();
    }
    lda8(row);
    for (int i = 0; i < 4; i++)
        lda8(asl8((u8)A));
    fc = 0;
    adc8(0x06);
    for (int k = 0; k < 6; k++) {
        wr8(ABS(0x1702 + k * 4), (u8)A);
        if (k < 5) lda8(inc8((u8)A));
    }
    lda8(0x21);
    for (int k = 0; k < 6; k++)
        wr8(ABS(0x1703 + k * 4), 0x21);
    for (int k = 0; k < 6; k++)
        wr8(ABS(0x1860 + k), 0);
    retl();
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
        wr8(ABS(0x1600 + Y + c * 4), (u8)A);
        wr8(ABS(0x160C + Y + c * 4), (u8)A);
        wr8(ABS(0x1618 + Y + c * 4), (u8)A);
        if (c < 2) { fc = 0; adc8(0x10); }
    }
    lda8(rd8(DP(0x00)));
    for (int r = 0; r < 3; r++) {
        wr8(ABS(0x1601 + Y + r * 12), (u8)A);
        wr8(ABS(0x1605 + Y + r * 12), (u8)A);
        wr8(ABS(0x1609 + Y + r * 12), (u8)A);
        if (r < 2) { fc = 0; adc8(0x10); }
    }
    u8 tile, nt, selpal;
    roster_select_portrait(st, i, &tile, &nt, &selpal);
    lda8(tile);
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            wr8(ABS(0x1602 + Y + r * 12 + c * 4), (u8)A);
            if (c < 2) { lda8(inc8((u8)A)); lda8(inc8((u8)A)); }
        }
        if (r < 2) { fc = 0; adc8(0x1C); }
    }
    u8 attr;
    lda8(rd8(DP(0x20)));
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
        push8(0x80); push16(0x8C8B); SelectName_m1x1();
        lda8(selpal);
        ora8(nt);
        attr = (u8)A;
    }
    for (int k = 0; k < 9; k++)
        wr8(ABS(0x1603 + Y + k * 4), attr);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02);
    for (int k = 0; k < 9; k++)
        wr8(ABS(0x1820 + Y + k), 0x02);
    X = pull8();
}

/*
 * $80:8BAE SelectDraw (RTL): which portrait is in front ($1579, from the
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
    int st = rd8(LONG(0x81800C)) != 0;
    for (int i = roster_count() - 1; i >= 5; i--)
        draw_portrait(i, cursor, st);
    for (int i = 4; i >= 0; i--) {
        X = (u8)i; nz8((u8)X);
        draw_portrait(i, cursor, st);
    }
    X = 0xFF; nz8(0xFF);
    retl();
}

/* $80:8D28 SelectHideRest (RTS): hide every portrait sprite but the front
 * portrait's (Y = $F0). */
void SelectHideRest_m1x1(void)
{
    ldx8(0x25);
    lda8(0xF0);
    u8 end = (u8)(1 + 36 * roster_count());          /* $B5 for five portraits */
    do {
        wr8(ABS(0x1600 + X), 0xF0);
        X = (u8)(X + 4);
        cmp8((u8)X, end);
    } while (!fz);
    ret();
}

/* $80:8D38 SelectSlideY (RTS): move the front portrait to row $198E (and
 * its name 4 pixels below it). */
void SelectSlideY_m1x1(void)
{
    ldx8(0x00);
    lda8(rd8(ABS(0x198E)));
    do {
        wr8(ABS(0x1601 + X), (u8)A);
        wr8(ABS(0x1605 + X), (u8)A);
        wr8(ABS(0x1609 + X), (u8)A);
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
        wr8(ABS(0x1701 + k * 4), (u8)A);
    ret();
}

/*
 * $80:8D6C SelectTransition (RTS): after the choice ($198B = 1) or on the
 * way in, count the timer $88 down (three per frame when leaving), slide the
 * front portrait (one row per frame towards $80:8D6A[$198B]) and hide the
 * others; at the target row the transition ends ($198D = 0; on the way in
 * also $90 = 0 and main screen $17).
 */
void SelectTransition_m1x1(void)
{
    rep(0x20);
    wr16(ABS(0x0088), dec16(rd16(ABS(0x0088))));
    X = rd8(ABS(0x198B)); nz8((u8)X);
    if (!fz) {
        wr16(ABS(0x0088), dec16(rd16(ABS(0x0088))));
        wr16(ABS(0x0088), dec16(rd16(ABS(0x0088))));
    }
    lda16(rd16(ABS(0x0088)));
    if (fn) {
        lda16(0x0001);
        wr16(ABS(0x0088), 0x0001);
    }
    sep(0x20);
    push8(0x80); push16(0x8D8C); SelectDraw_m1x1();
    push16(0x8D8F); SelectSlideY_m1x1();
    push16(0x8D92); SelectHideRest_m1x1();
    X = rd8(ABS(0x198B));
    cmp8((u8)X, 0x01);
    if (fz)
        wr8(ABS(0x198E), dec8(rd8(ABS(0x198E))));
    else
        wr8(ABS(0x198E), inc8(rd8(ABS(0x198E))));
    lda8(rd8(ABS(0x198E)));
    cmp8((u8)A, rd8(ABS(0x8D6A + X)));
    if (fz) {
        lda8(rd8(ABS(0x198B)));
        if (fz) {
            wr8(DP(0x90), 0);
            lda8(0x17);
            wr8(ABS(0x1905), 0x17);
        }
        wr8(ABS(0x198D), 0);
    }
    ret();
}

/*
 * $80:8B26 SelectTimer (RTS): the countdown in seconds (timer $88 / 60 with
 * the hardware divider) as two digits at VRAM $1A8F (list at $03CF: tiles
 * $00-$09 on top, $10-$19 below).
 */
void SelectTimer_m1x1(void)
{
    lda8(rd8(DP(0x88))); wr8(ABS(0x4204), (u8)A);
    lda8(rd8(DP(0x89))); wr8(ABS(0x4205), (u8)A);
    lda8(0x3C); wr8(ABS(0x4206), 0x3C);
    wr8(DP(0x00), 0);
    lda8(rd8(ABS(0x4214)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        wr8(DP(0x00), inc8(rd8(DP(0x00))));
        WAIT_TICK();
    }
    wr8(ABS(0x03D1), (u8)A);
    ora8(0x10);
    wr8(ABS(0x03D3), (u8)A);
    lda8(rd8(DP(0x00)));
    wr8(ABS(0x03D0), (u8)A);
    ora8(0x10);
    wr8(ABS(0x03D2), (u8)A);
    lda8(0x22);
    wr8(ABS(0x03CF), 0x22);
    rep(0x20);
    lda16(0x1A8F); wr16(DP(0x00), 0x1A8F);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    push16(0x8B73); VramQueueTiles20_m1x1();
    ret();
}

/* Voice samples for the chosen character: $82:8593 with X = character + 1,
 * or the roster's stream for an added character (port only). */
void SpcUploadB_m1x1(void);
void SpcUploadFrom(u32 stream);

/*
 * $80:89D7 SelectMain (RTS), every frame of the player select (mode 1).
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
    lda8(rd8(LONG(0x81800E)));
    if (!fz) {
        lda8(rd8(DP(0x21)));
        and8(0x10);
        if (!fz) goto next_segment;
    }
    lda8(rd8(ABS(0x198D)));
    if (!fz) {
        wr8(DP(0x20), 0);
        push16(0x89EC); SelectTransition_m1x1();
        goto tail;
    }
    lda8(rd8(ABS(0x198F)));
    if (!fz) {
        rep(0x20);
        wr16(ABS(0x0088), dec16(rd16(ABS(0x0088))));
        lda16(rd16(ABS(0x0088)));
        cmp16(A, 0x0001);
        if (fn) {
            lda16(0x0001);
            wr16(ABS(0x0088), 0x0001);
        }
        sep(0x20);
        wr8(ABS(0x198F), dec8(rd8(ABS(0x198F))));
        wr8(DP(0x20), 0);
        push8(0x80); push16(0x8A12); SelectDraw_m1x1();
        lda8(rd8(ABS(0x198B)));
        if (!fz) {
            push16(0x8A1A); SelectSlideY_m1x1();
            push16(0x8A1D); SelectHideRest_m1x1();
        }
        ret();
        return;
    }
    lda8(rd8(ABS(0x198B)));
    if (!fz) {
        lda8(rd8(LONG(0x7FFFFF)));
        cmp8((u8)A, 0x06);
        if (fc) {
            if (!fz) goto next_segment;
            rep(0x20);
            lda16(rd16(LONG(0x7FFFFA)));
            cmp16(A, 0x0708);
            sep(0x20);
            if (fc) goto next_segment;
        }
        rep(0x20);
        wr16(ABS(0x0088), dec16(rd16(ABS(0x0088))));
        sep(0x20);
        if (fz) goto next_segment;
        lda8(rd8(DP(0x21)));
        and8(0x80);
        if (!fz) {
            lda8(0x3D);
            push8(0x80); push16(0x8A54); QueueSoundB_m1x1();
            lda8(0x1E);
            wr8(ABS(0x198F), 0x1E);
            wr8(ABS(0x198B), 0);
            wr8(ABS(0x198D), inc8(rd8(ABS(0x198D))));
        }
        wr8(DP(0x20), 0);
        push8(0x80); push16(0x8A65); SelectDraw_m1x1();
        push16(0x8A68); SelectSlideY_m1x1();
        push16(0x8A6B); SelectHideRest_m1x1();
        goto tail;
    }
    rep(0x20);
    lda16(rd16(ABS(0x0088)));
    cmp16(A, 0x0001);
    sep(0x20);
    if (fz)
        wr8(DP(0x20), 0);
    lda8(rd8(ABS(0x1E8B)));
    wr8(DP(0x1F), (u8)A);
    push8(0x80); push16(0x8A85); SelectRotate_m1x1();
    push8(0x80); push16(0x8A89); SelectDraw_m1x1();
    rep(0x20);
    wr16(ABS(0x0088), dec16(rd16(ABS(0x0088))));
    sep(0x20);
    if (fz) {
        lda8(rd8(ABS(0x1701)));
        cmp8((u8)A, 0xF0);
        if (fz) {
            lda8(0x01);
            wr8(ABS(0x0088), 0x01);
            goto tail;
        }
    } else {
        lda8(rd8(DP(0x26)));
        and8(0x80);
        if (fz) goto tail;
        lda8(rd8(ABS(0x1559)));
        if (!fz) goto tail;
    }
    /* the choice */
    wr8(DP(0x90), inc8(rd8(DP(0x90))));
    wr8(ABS(0x198B), inc8(rd8(ABS(0x198B))));
    lda8(0x15);
    wr8(ABS(0x1905), 0x15);
    wr8(ABS(0x198D), inc8(rd8(ABS(0x198D))));
    lda8(0x1E);
    wr8(ABS(0x198F), 0x1E);
    {
        X = rd8(ABS(0x1579));
        u8 ch = roster_select_char(X);
        u8 ech = roster_engine_char(ch);
        lda8(ech);
        wr8(ABS(0x154F), ech);
        roster_note_choice(ch);
        wr8(ABS(0x4200), 0);
        rep(0x20);
        for (int x = 0x3E; x >= 0; x -= 2) {
            wr16(ABS(0x1400 + x), 0);
            wr16(ABS(0x1440 + x), 0);
            wr16(ABS(0x1480 + x), 0);
            wr16(ABS(0x14C0 + x), 0);
        }
        X = 0xFE;
        sep(0x20);
        static const u8 cleared[] = { 0x69, 0x65, 0x6D, 0x6A, 0x66, 0x6E, 0x6B, 0x67, 0x6F, 0x6C, 0x68, 0x70 };
        for (unsigned k = 0; k < sizeof cleared; k++)
            wr8(DP(cleared[k]), 0);
        u32 stream = roster_voice_stream(ch);
        X = rd8(ABS(0x154F));
        X = inc8((u8)X);
        if (stream) {
            SpcUploadFrom(stream);
        } else {
            push8(0x80); push16(0x8B03); SpcUploadB_m1x1();
        }
        lda8(0x3F);
        push8(0x80); push16(0x8B0D); QueueSoundB_m1x1();
        lda8(rd8(ABS(0x190E)));
        wr8(ABS(0x4200), (u8)A);
    }
tail:
    push16(0x8B16); SelectTimer_m1x1();
    push8(0x80); push16(0x8B1A); RandomB_m1x1();
    and8(0x01);
    wr8(ABS(0x1E91), (u8)A);
    ret();
    return;

next_segment:
    /* $80:8E51: course and final flag from the broadcast minute, then the
     * course map (mode $18) or the jump contest (mode $17); a final
     * continues at $80:8F15. */
    {
        ldx8(0xFF);
        wr8(DP(0x00), 0xFF);
        lda8(rd8(LONG(0x7FFFFF)));
        do {
            wr8(DP(0x00), inc8(rd8(DP(0x00))));
            X = inc8((u8)X);
            cmp8((u8)A, rd8(LONG(0x808E48 + X)));
            WAIT_TICK();
        } while (fc);
        cmp8((u8)X, 0x04);
        if (fz) {
            lda8(0x17);
            wr8(DP(0x76), 0x17);
            r816_call_pc(0x808F17);
            return;
        }
        if (!fc)
            ;
        else
            wr8(DP(0x00), dec8(rd8(DP(0x00))));
        lda8(rd8(DP(0x00)));
        lda8(lsr8((u8)A));
        wr8(DP(0x7C), (u8)A);
        lda8(rol8((u8)A));
        and8(0x01);
        wr8(DP(0x7D), (u8)A);
        if (!fz) {
            r816_call_pc(0x808F15);
            return;
        }
        lda8(0x18);
        wr8(DP(0x76), 0x18);
        r816_call_pc(0x808F17);
    }
}
