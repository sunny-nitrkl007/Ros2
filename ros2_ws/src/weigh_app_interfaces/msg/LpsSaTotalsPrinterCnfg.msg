# Wire equivalent of LpsSaTotalsPrinterCnfg (nested struct, the actual
# config payload of the PrinterCnfgInput channel -- see
# LpsSaTotalsPrinterCnfgInterface.msg for the channel wrapper).
# Source: CPM-Loader-ais/prod/common/interfaces/LpsSaTotals/PrinterCnfg.hpp:31-138
# Field presence cross-checked against serialize() (lines 101-137) at
# current version 3 -- every declared member is archived.
#
# LpsSaTotalsPrinterCnfgDateFormat/DateSeparator (PrinterCnfg.hpp:11-27):
# explicit uint16_t enum values, not blocked.

uint16 DATE_FORMAT_YYYY_MM_DD=0x0670
uint16 DATE_FORMAT_MM_DD_YYYY=0x0671
uint16 DATE_FORMAT_DD_MM_YYYY=0x0672
uint16 DATE_FORMAT_YY_MM_DD=0x0673
uint16 DATE_FORMAT_MM_DD_YY=0x0674
uint16 DATE_FORMAT_DD_MM_YY=0x0675
uint16 DATE_FORMAT_MM_DD=0x0676
uint16 DATE_FORMAT_DD_MM=0x0677
uint16 date_format

uint16 DATE_SEPARATOR_SLASH=0x0678
uint16 DATE_SEPARATOR_HYPHEN=0x0679
uint16 date_separator

bool installed
uint8 leading_blank_lines
uint8 trailing_blank_lines

LpsSaTotalsPrinterCnfgTruckTicket truck_ticket
LpsSaTotalsPrinterCnfgTruckReport truck_report
LpsSaTotalsPrinterCnfgMaterialReport material_report

string udp_transfer_ip
uint32 udp_transfer_port
bool udp_transfer_enabled
