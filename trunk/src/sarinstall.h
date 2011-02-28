#ifndef SARINSTALL_H
#define SARINSTALL_H

#include "sar.h"

extern Boolean SARIsInstalledLocal(
        sar_dname_struct *dn,
        sar_fname_struct *fn
);
extern Boolean SARIsInstalledGlobal(
        sar_dname_struct *dn,
        sar_fname_struct *fn
);

extern int SARDoInstallLocal(
        sar_dname_struct *dn,
        sar_fname_struct *fn,
        sar_option_struct *opt
);

#endif	/* SARINSTALL_H */
