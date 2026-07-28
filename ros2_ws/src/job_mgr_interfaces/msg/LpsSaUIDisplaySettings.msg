# Wire equivalent of LpsSaUIDisplaySettings.
# Source: prod/common/interfaces/LpsSaUI/DisplaySettings.hpp
#
# 5 of the 7 enums here are locally defined in this same header (lines
# 12-108) -- values fully verified, not blocked. weight_units and
# weight_precision reference LpsCommonWeightUnits/LpsCommonWeightPrecision
# (LpsCommonWeight.h, missing from this checkout) -- same blocked situation
# as LpsSaLoadRecordChannel.msg's weight_units. Confirmed real enumerator
# names for LpsCommonWeightPrecision (not values): STANDARD, MEDIUM, HIGH
# (DisplaySettings.hpp:120,147,152).
#
# serialize() (lines 197-219) also archives a placeholder int8 "older
# versions had weight resolution in this place" that's always written as the
# constant 1 and never read back into a real field -- a backward-compat
# artifact, not real data, deliberately not included here.

uint8 UNITS_METRIC=0
uint8 UNITS_ENGLISH=1
uint8 units

# LpsSaUIDisplaySettingsLanguage (DisplaySettings.hpp:19-83) -- 63 values,
# all explicit hex literals, fully verified.
uint16 LANG_ENGLISH=0
uint16 LANG_SPANISH=1
uint16 LANG_FRENCH=2
uint16 LANG_PORTUGUESE=3
uint16 LANG_GERMAN=4
uint16 LANG_NORWEGIAN=5
uint16 LANG_SWEDISH=6
uint16 LANG_FINNISH=7
uint16 LANG_DUTCH=8
uint16 LANG_DANISH=9
uint16 LANG_ITALIAN=10
uint16 LANG_GREEK=11
uint16 LANG_TURKISH=12
uint16 LANG_INDONESIAN=13
uint16 LANG_RUSSIAN=14
uint16 LANG_JAPANESE=15
uint16 LANG_HUNGARIAN=16
uint16 LANG_ICELANDIC=17
uint16 LANG_ARABIC=18
uint16 LANG_TRADITIONAL_CHINESE=19
uint16 LANG_CZECH=20
uint16 LANG_POLISH=21
uint16 LANG_SIMPLIFIED_CHINESE=22
uint16 LANG_THAI=23
uint16 LANG_SLOVENIAN=24
uint16 LANG_SLOVAKIAN=25
uint16 LANG_ESTONIAN=26
uint16 LANG_LATVIAN=27
uint16 LANG_LITHUANIAN=28
uint16 LANG_MALAYSIAN=29
uint16 LANG_KOREAN=30
uint16 LANG_PORTUGUESE_BRAZIL=31
uint16 LANG_FARSI=32
uint16 LANG_HEBREW=33
uint16 LANG_SERBIAN=34
uint16 LANG_CROATIAN=35
uint16 LANG_BULGARIAN=36
uint16 LANG_ROMANIAN=37
uint16 LANG_VIETNAMESE=38
uint16 LANG_MONGOLIAN=39
uint16 LANG_MACEDONIAN=40
uint16 LANG_BURMESE=41
uint16 LANG_KHMER=42
uint16 LANG_UKRAINIAN=43
uint16 LANG_SPANISH_CASTILIAN_EAME=44
uint16 LANG_FRENCH_EAME=45
uint16 LANG_PORTUGUESE_EAME=46
uint16 LANG_DUTCH_NETHERLANDS=47
uint16 LANG_BURMESE_MYANMASA=48
uint16 LANG_AFRIKAANS=49
uint16 LANG_ALBANIAN=50
uint16 LANG_ARMENIAN=51
uint16 LANG_BOSNIAN=52
uint16 LANG_DARI=53
uint16 LANG_DUTCH_BELGIUM=54
uint16 LANG_FRENCH_CANADIAN=55
uint16 LANG_HINDI=56
uint16 LANG_KAZAKH=57
uint16 LANG_LAOTIAN=58
uint16 LANG_SPANISH_LACD=59
uint16 LANG_TAMIL=60
uint16 LANG_TURKMEN=61
uint16 LANG_URDU=62
uint16 language

uint8 brightness

uint8 TIME_FORMAT_24HR=0
uint8 TIME_FORMAT_12HR=1
uint8 time_format

uint16 DATE_FORMAT_YYYY_MM_DD=1648
uint16 DATE_FORMAT_MM_DD_YYYY=1649
uint16 DATE_FORMAT_DD_MM_YYYY=1650
uint16 date_format

int32 weight_units       # LpsCommonWeightUnits -- blocked

int32 weight_precision   # LpsCommonWeightPrecision -- blocked

uint16 service_mode_enable_code

uint8 KEYBOARD_QWERTY=0
uint8 KEYBOARD_NATIVE=1
uint8 KEYBOARD_MIXED=2
uint8 keyboard_layout_setting
