/**********************************************************************
*   This file is part of Search and Rescue II (SaR2).                 *
*                                                                     *
*   SaR2 is free software: you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License v.2 as       *
*   published by the Free Software Foundation.                        *
*                                                                     *
*   SaR2 is distributed in the hope that it will be useful, but       *
*   WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See          *
*   the GNU General Public License for more details.                  *
*                                                                     *
*   You should have received a copy of the GNU General Public License *
*   along with SaR2.  If not, see <http://www.gnu.org/licenses/>.     *
***********************************************************************/

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
