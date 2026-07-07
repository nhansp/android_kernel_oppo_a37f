/*
 * oppo_glue.c - opchg <-> qpnp glue for the OPPO A37f (msm8916)
 *
 * The stock oppo-a37 kernel places a handful of helper symbols inside its
 * *forked* qpnp-linear-charger.c (the PMIC linear charger). The A37f does not
 * use the PMIC linear charger - charging is performed by the external TI
 * BQ24196 charger IC on i2c 0x6b - so CONFIG_QPNP_LINEAR_CHARGER is disabled
 * and the linear-charger handle "the_chip" is never populated. The opchg code
 * only NULL-checks the_chip, so reproducing that NULL handle plus the small
 * wrappers below yields the same behaviour the stock A37f exhibits, while
 * sourcing charger presence from the authoritative BQ24196 chip layer.
 *
 * opchg_backup_ocv_soc() is provided separately in qpnp-vm-bms.c (the fuel
 * gauge the A37f actually uses).
 */

#include <oppo_inc.h>

/* PMIC linear-charger handle - unused on the A37f (external BQ24196) -> NULL */
struct qpnp_lbc_chip *the_chip = NULL;

/*
 * Charger in/out. On stock this read the PMIC linear charger; on the A37f the
 * authoritative view of "is a charger attached" is the BQ24196 chip state.
 */
int opchg_get_charger_inout(void)
{
	if (opchg_chip)
		return opchg_chip->chg_present ? 1 : 0;
	return 0;
}

/*
 * PMIC SoC scratch memory. On stock this lived in the linear-charger PMIC BMS
 * data register and is consumed only by the bq27541 VOOC fuel gauge, which the
 * A37f does not have. Keep a RAM shadow so that (compiled but inactive) path
 * stays coherent.
 */
static int opchg_pmic_soc_shadow;

void opchg_set_pmic_soc_memory(int soc)
{
	opchg_pmic_soc_shadow = soc;
}

int opchg_get_pmic_soc_memory(void)
{
	return opchg_pmic_soc_shadow;
}
