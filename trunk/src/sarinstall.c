#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../include/disk.h"

#include "sar.h"
#include "optionio.h"
#include "sarinstall.h"


Boolean SARIsInstalledLocal(
	sar_dname_struct *dn,
	sar_fname_struct *fn
);
Boolean SARIsInstalledGlobal(
	sar_dname_struct *dn,
	sar_fname_struct *fn
);

int SARDoInstallLocal(
	sar_dname_struct *dn,
        sar_fname_struct *fn,
        sar_option_struct *opt
);


/*
 *	Checks if the program's compoent appears to be installed locally,
 *	returns true if it is installed.
 *
 *	The input path structures need to be specified.
 */
Boolean SARIsInstalledLocal(
	sar_dname_struct *dn,
	sar_fname_struct *fn
)
{
	struct stat stat_buf;


	if((dn == NULL) || (fn == NULL))
	    return(False);

	/* Nessasary paths specified? */
	if((*(dn->local_data) == '\0') ||
           (*(fn->options) == '\0')
	)
	    return(False);


	/* Local directory exists? */
	if(stat(dn->local_data, &stat_buf))
	    return(False);

	/* Is a directory? */
#ifdef S_ISDIR
	if(!S_ISDIR(stat_buf.st_mode))
	    return(False);
#endif	/* S_ISDIR */

	/* Configuration file exists? */
	if(stat(fn->options, &stat_buf))
	    return(False);

	/* Is a file? */
#ifdef S_ISREG
	if(!S_ISREG(stat_buf.st_mode))
	    return(False);
#endif	/* S_ISREG */

	return(True);
}

/*
 *      Checks if the program's compoent appears to be installed globally,
 *      returns true if it is installed.
 *
 *	The input path structures need to be specified.
 */
Boolean SARIsInstalledGlobal(
        sar_dname_struct *dn,
        sar_fname_struct *fn
)
{
	struct stat stat_buf;


	if((dn == NULL) || (fn == NULL))
	    return(False);

	/* Nessasary paths specified? */
	if(*(dn->global_data) == '\0')
	    return(False);


	/* Global directory exists? */
	if(stat(dn->global_data, &stat_buf))
	    return(False);

	/* Is a directory? */
#ifdef S_ISDIR
	if(!S_ISDIR(stat_buf.st_mode))
	    return(False);
#endif	/* S_ISDIR */

        return(True);
}  


/*
 *	Procedure to install program files locally.
 *
 *	Returns non-zero on error.
 */
int SARDoInstallLocal(
	sar_dname_struct *dn,
	sar_fname_struct *fn,
	sar_option_struct *opt
)
{
	struct stat stat_buf;


	if((dn == NULL) || (fn == NULL))
	    return(-1);

	/* Create local program directory as meeded */
	if(stat(dn->local_data, &stat_buf))
	{
	    if(rmkdir(
		dn->local_data,
#ifdef __MSW__
		0
#else
		S_IRUSR | S_IWUSR | S_IXUSR	/* User rwx only */
#endif
	    ))
		return(-1);
	}

	/* Create default configuration file */
	if(SAROptionsSaveToFile(opt, fn->options))
	    return(-1);

	return(0);
}

