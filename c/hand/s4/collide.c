/*
 * Stadium 3-4 (Gen B): rider-to-rider collisions and coins, hand-written.
 */
#include "hand/hand.h"
#include "ram.h"
#include "roster.h"

void QueueSoundB_m1x1(void);

/* $80:C989 Reach20 (RTS): A + $14 unless A >= $E7 (then A unchanged). */
void Reach20_m1x1(void)
{
    cmp8((u8)A, 0xE7);
    if (!fc)
        adc8(0x14);
    ret();
}

/* $80:C990 Reach2 (RTS): A + 2 unless A >= $E7. */
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
        wr8(ABS(BANNER_COUNT), (u8)A);
        lda8(0x01);
        wr8(ABS(SCORE_ADD + 0x01), 0x01);
        wr8(ABS(BANNER_ON), 0x01);
        lda8(0x40);
        wr8(ABS(BANNER_TIMER), 0x40);
        lda8(rd8(ABS(RIDER_Y)));
        fc = 1;
        sbc8(0x10);
        wr8(ABS(BANNER_Y), (u8)A);
        lda8(0x2B);
        push8(0x81); push16(0xC722); QueueSoundB_m1x1();
    }
    retl();
}

/*
 * $80:D08A CollectCoin (RTL): sound $31; take a free coin effect slot
 * ($1530,X = 0, X = 0, 2, 4, 6) or, with all four busy, the oldest
 * ($1538,X largest, counted in $1541) and start it at the player ($01B6 + 16;
 * speed $F8/$E0, timer $38); count the coin ($1542, milestone check), the
 * combo $1540 (up to 5) adds $80:D11D/$80:D124[combo] to the score digits
 * $09AF/$09B0, and $1543/$1543,combo count up.
 */
void CollectCoin_m1x1(void)
{
    lda8(0x31);
    push8(0x80); push16(0xD08F); QueueSoundB_m1x1();
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
    push8(0x80); push16(0xD0EC); CoinMilestone_m1x1();
    lda8(rd8(ABS(COIN_COMBO)));
    cmp8((u8)A, 0x05);
    if (fc) {
        ldy8(0x05);
    } else {
        wr8(ABS(COIN_COMBO), inc8(rd8(ABS(COIN_COMBO))));
        ldy8(rd8(ABS(COIN_COMBO)));
    }
    lda8(rd8(ABS(0xD11D + Y)));
    fc = 0;
    adc8(rd8(ABS(SCORE_ADD + 0x03)));
    wr8(ABS(SCORE_ADD + 0x03), (u8)A);
    lda8(rd8(ABS(0xD124 + Y)));
    fc = 0;
    adc8(rd8(ABS(SCORE_ADD + 0x04)));
    wr8(ABS(SCORE_ADD + 0x04), (u8)A);
    wr8(ABS(COMBO_COUNTS), inc8(rd8(ABS(COMBO_COUNTS))));
    lda8(rd8(ABS(COMBO_COUNTS + Y)));
    lda8(inc8((u8)A));
    wr8(ABS(COMBO_COUNTS + Y), (u8)A);
    retl();
}

/* JSL QueueSoundB from bank $80 with the return address `ret`. */
static void sound80(u8 cmd, u16 ret)
{
    lda8(cmd);
    push8(0x80); push16(ret); QueueSoundB_m1x1();
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

/*
 * $80:C776 RiderCollisions (RTS): every pair of riders X < Y that are both
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
        push16(0xC7C0); Reach20_m1x1();
        cmp8((u8)A, rd8(ABS(RIDER_Y + Y)));
        if (fc) goto side;
        goto next_y;
    }
    lda8(rd8(ABS(RIDER_Y + Y)));
    push16(0xC7CE); Reach20_m1x1();
    cmp8((u8)A, rd8(ABS(RIDER_Y + X)));
    if (!fc) goto next_y;
side:
    lda8(rd8(ABS(RIDER_X + X)));
    cmp8((u8)A, rd8(ABS(RIDER_X + Y)));
    if (!fc) {
        push16(0xC7DE); Reach2_m1x1();
        cmp8((u8)A, rd8(ABS(RIDER_X + Y)));
        if (fc) goto bump_both;
        lda8(rd8(ABS(RIDER_X + X)));
        push16(0xC7E9); Reach20_m1x1();
        cmp8((u8)A, rd8(ABS(RIDER_X + Y)));
        if (fc) goto rear_bump;
        goto next_y;
    }
    lda8(rd8(ABS(RIDER_X + Y)));
    push16(0xC7F6); Reach2_m1x1();
    cmp8((u8)A, rd8(ABS(RIDER_X + X)));
    if (fc) goto bump_both;
    lda8(rd8(ABS(RIDER_X + Y)));
    push16(0xC801); Reach20_m1x1();
    cmp8((u8)A, rd8(ABS(RIDER_X + X)));
    if (fc) goto meet;
    goto next_y;

rear_bump:
    crash_sound((u8)X, 0xC815);
    cmp8((u8)X, 0x00);
    if (fz) sound80(0x3D, 0xC81F);
    lda8(0xFF);
    wr8(ABS(RIDER_BUMPED + X), 0xFF);
    goto after_bump;

bump_both:
    crash_sound((u8)X, 0xC833);
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
                sound80(0x29, 0xC85E);
                push8((u8)Y);
                push8((u8)X);
                push8(0x80); push16(0xC864); CollectCoin_m1x1();
                X = pull8();
                Y = pull8(); nz8((u8)Y);
                lda8(rd8(ABS(COIN_COUNT)));
                cmp8((u8)A, 0x05);
                if (fz) {
                    sound80(0x2D, 0xC87C);
                    lda8(0x01);
                    wr8(ABS(TURBO_ON), 0x01);
                    push8(0x80); push16(0xC885); QueueSoundB_m1x1();
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
        push8(0x80); push16(0xC8BF); QueueSoundB_m1x1();
    }
    Y = pull8();
    crash_sound((u8)Y, 0xC8CC);
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
