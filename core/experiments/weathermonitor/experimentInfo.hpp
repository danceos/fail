#ifndef __WEATHERMONITOR_EXPERIMENT_INFO_HPP__
#define __WEATHERMONITOR_EXPERIMENT_INFO_HPP__

// FIXME autogenerate this

#define GUARDED_WEATHERMONITOR 0

#if !GUARDED_WEATHERMONITOR // without vptr guards

// main() address:
// nm -C vanilla.elf|fgrep main
#define WEATHER_FUNC_MAIN			0x001010f0 
// Temperature::measure() address:
// nm -C vanilla.elf|fgrep 'Temperature::measure()'
#define WEATHER_FUNC_TEMP_MEASURE	0x001010f0 
// number of instructions we want to observe
#define WEATHER_NUMINSTR			10000
// data/BSS begin:
// nm -C vanilla.elf|fgrep ___DATA_START__
#define WEATHER_DATA_START			0x00101c34
// data/BSS end:
// nm -C vanilla.elf|fgrep ___BSS_END__
#define WEATHER_DATA_END			0x00103529
// text begin:
// nm -C vanilla.elf|fgrep ___TEXT_START__
#define WEATHER_TEXT_START			0x00100000
// text end:
// nm -C vanilla.elf|fgrep ___TEXT_END__
#define WEATHER_TEXT_END			0x00101a5b

#else // with guards

// XXX

#endif

#endif
