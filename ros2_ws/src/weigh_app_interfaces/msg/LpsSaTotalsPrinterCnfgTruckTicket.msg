# Wire equivalent of LpsSaTotalsPrinterCnfg::TruckTicket (nested struct,
# not its own channel).
# Source: CPM-Loader-ais/prod/common/interfaces/LpsSaTotals/PrinterCnfg.hpp:52-83
# Field presence cross-checked against LpsSaTotalsPrinterCnfg::serialize()
# (PrinterCnfg.hpp:101-137) at current version 3 -- every declared member is
# archived (footerLine1 at v>=2, retentionPeriod at v>=3), so the full
# struct is captured.
#
# retentionPeriod's RETENTION_PERIOD_DAYS_DEFAULT (732) is a static
# constexpr default, not wire data -- not carried here.

uint8 num_copies
bool header_enabled
string header_line1
string header_line2
string header_line3
string footer_line1
bool bucket_weights_enabled
bool truck_weight_enabled
bool truck_id_enabled
bool material_id_enabled
bool store_time_enabled
uint32 ticket_number
uint16 retention_period
