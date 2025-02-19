// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese
/************************************************************************************************
  Poker Monarch

  GFX ROMs contain
  'Extrema Systems International Ltd'
  as well as a logo for the company.

  There are also 'Lucky Boy' graphics in various places.

  * Turn on all the dips of SW1
  * Restart game
  * If it says ERROR OF RAM GAME STOP, press F2
  * When you get a blank blue screen Press Alt+2
  * This gives a setup screen. Press F2 to see cards and logo (and it beeps)
  * Depending on settings of SW1, you can get other cards, or other test screens.

*************************************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class poker72_state : public driver_device
{
public:
	poker72_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_vram(*this, "vram"),
		m_pal(*this, "pal"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_rombank(*this, "rombank")
	{ }

	void poker72(machine_config &config);

	void init_poker72();

protected:
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_pal;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_memory_bank m_rombank;

	uint8_t m_tile_bank;

	void paletteram_w(offs_t offset, uint8_t data);
	void output_w(uint8_t data);
	void tile_bank_w(uint8_t data);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map);
};


void poker72_state::video_start()
{
	m_tile_bank = 0;

	save_item(NAME(m_tile_bank));
}

uint32_t poker72_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int count = 0;

	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			int tile = ((m_vram[count + 1] & 0x0f) << 8 ) | (m_vram[count+0] & 0xff); //TODO: tile bank
			int fx = (m_vram[count + 1] & 0x10);
			int fy = (m_vram[count + 1] & 0x20);
			int color = (m_vram[count + 1] & 0xc0) >> 6;

			tile|= m_tile_bank << 12;

			m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, tile, color, fx, fy, x * 8, y * 8);

			count += 2;
		}
	}

	return 0;
}

void poker72_state::paletteram_w(offs_t offset, uint8_t data)
{
	m_pal[offset] = data;

	int const r = m_pal[(offset & 0x3ff) + 0x000] & 0x3f;
	int const g = m_pal[(offset & 0x3ff) + 0x400] & 0x3f;
	int const b = m_pal[(offset & 0x3ff) + 0x800] & 0x3f;

	m_palette->set_pen_color( offset & 0x3ff, pal6bit(r), pal6bit(g), pal6bit(b));
}

void poker72_state::output_w(uint8_t data)
{
	logerror("output_w: %02x\n", data);

/*  if ((data & 0xc) == 0xc)
        m_rombank->set_entry(2);
    else*/
	if (data & 8)
		m_rombank->set_entry(1);
	else
		m_rombank->set_entry(0);
}

void poker72_state::tile_bank_w(uint8_t data)
{
	m_tile_bank = (data & 4) >> 2;
}

void poker72_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr(m_rombank);
	map(0xc000, 0xdfff).ram(); //work ram
	map(0xe000, 0xefff).ram().share(m_vram);
	map(0xf000, 0xfbff).ram().w(FUNC(poker72_state::paletteram_w)).share(m_pal);
	map(0xfc00, 0xfdff).ram(); //???
	map(0xfe08, 0xfe08).portr("SW1");
	map(0xfe09, 0xfe09).portr("IN1");
	map(0xfe0a, 0xfe0a).portr("IN2");
	map(0xfe0c, 0xfe0c).portr("SW4");
	map(0xfe0d, 0xfe0d).portr("SW5");
	map(0xfe0e, 0xfe0e).portr("SW6");

	map(0xfe17, 0xfe17).nopr(); //irq ack
	map(0xfe20, 0xfe20).w(FUNC(poker72_state::output_w)); //output, irq enable?
	map(0xfe22, 0xfe22).w(FUNC(poker72_state::tile_bank_w));
	map(0xfe40, 0xfe40).rw("ay", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xfe60, 0xfe60).w("ay", FUNC(ay8910_device::address_w));

	map(0xff00, 0xffff).ram(); //??
/*
bp 13a

fe06 w
fe08 w
fe0b = 9b (ppi?)
fe24 w

01f9 : call 6399 --> cls

*/
}


static INPUT_PORTS_START( poker72 )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x00, "SW1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) // Z
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) // X
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) // C
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 ) // V
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 ) // B
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("M. Bet")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Black")


	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Red")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_GAMBLE_DEAL ) // '2'
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) // M
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SERVICE1 ) // '9'
	PORT_SERVICE( 0x0080, IP_ACTIVE_HIGH ) // F2

	PORT_START("SW4")
	PORT_DIPNAME( 0x01, 0x00, "SW4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_START("SW5")
	PORT_DIPNAME( 0x01, 0x00, "SW5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_START("SW6")
	PORT_DIPNAME( 0x01, 0x00, "SW6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )


	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x00, "SW2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SW3")
	PORT_DIPNAME( 0x01, 0x00, "SW3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,4),
	8,
	{ RGN_FRAC(3,4), RGN_FRAC(3,4)+4, RGN_FRAC(2,4), RGN_FRAC(2,4)+4 ,RGN_FRAC(1,4),RGN_FRAC(1,4)+4, RGN_FRAC(0,4),RGN_FRAC(0,4)+4 },
	{ 0,1,2,3,8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};



static GFXDECODE_START( gfx_poker72 )
	GFXDECODE_ENTRY( "tiles", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

// default 444 palette for debug purpose
void poker72_state::palette(palette_device &palette) const
{
	for (int x = 0; x < 0x100; x++)
	{
		int const r = (x & 0x0f);
		int const g = (x & 0x3c) >> 2;
		int const b = (x & 0xf0) >> 4;
		palette.set_pen_color(x, rgb_t(pal4bit(r), pal4bit(g), pal4bit(b)));
	}
}

void poker72_state::machine_reset()
{
	m_rombank->set_entry(0);
}

void poker72_state::poker72(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8000000);         // ? MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &poker72_state::prg_map);
	m_maincpu->set_vblank_int("screen", FUNC(poker72_state::irq0_line_hold));

	I80C51(config, "subcpu", 8000000); // actually 89C51, ? MHz

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(poker72_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_poker72);
	PALETTE(config, m_palette, FUNC(poker72_state::palette), 0xe00);

	SPEAKER(config, "mono").front_center();

	ay8910_device &ay(AY8910(config, "ay", 8000000 / 8)); // ? Mhz
	ay.port_a_read_callback().set_ioport("SW2");
	ay.port_b_read_callback().set_ioport("SW3");
	ay.add_route(ALL_OUTPUTS, "mono", 0.50);
}



ROM_START( poker72 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "27010.bin", 0x00000, 0x20000, CRC(62447341) SHA1(e442c1f834a5dd2ab6ab3bdd316dfa86f2ca6647) )

	ROM_REGION( 0x1000, "subcpu", 0 )
	ROM_LOAD( "89c51.bin", 0x00000, 0x1000, CRC(3fdd2148) SHA1(ea39a52482967268c7387aec77cfab1ae5c427fa) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "270135.bin", 0x00000, 0x20000, CRC(188c96ee) SHA1(7e883454cb080cdc82ce47ac92f51c8d45a55085) )
	ROM_LOAD( "270136.bin", 0x20000, 0x20000, CRC(f84c5068) SHA1(49178fe7b12f547a50879002236105a882767ebb) )
	ROM_LOAD( "270137.bin", 0x40000, 0x20000, CRC(310281d1) SHA1(c28f97bb3613c0b481ab6e16e215549c44b83c47) )
	ROM_LOAD( "270138.bin", 0x60000, 0x20000, CRC(d689313d) SHA1(8b9661b3af0e2ced7fe9fa487641e445ce7835b8) )
ROM_END

void poker72_state::init_poker72()
{
	uint8_t *rom = memregion("maincpu")->base();

	// configure and initialize bank 1
	m_rombank->configure_entries(0, 4, memregion("maincpu")->base(), 0x8000);
	m_rombank->set_entry(0);

	//rom[0x4a9] = 0x28;
	rom[0x4aa] = 0x00;
}

} // Anonymous namespace


GAME( 1995, poker72,  0,    poker72, poker72, poker72_state, init_poker72, ROT0, "Extrema Systems International Ltd.", "Poker Monarch (v2.50)", MACHINE_NOT_WORKING )
