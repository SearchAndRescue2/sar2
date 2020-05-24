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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "sfm.h"
#include "sarreality.h"

SFMRealmStruct *SFMInit(int argc, char **argv);
void SFMShutdown(SFMRealmStruct *realm);

void SFMSetTiming(SFMRealmStruct *realm, SFMTime lapsed_ms);
void SFMSetTimeCompression(SFMRealmStruct *realm, double compression);
void SFMUpdateRealm(SFMRealmStruct *realm, SFMTime lapsed_ms);


/*
 *	Creates a new FDM Realm.
 */
SFMRealmStruct *SFMInit(int argc, char **argv)
{
	SFMRealmStruct *realm = SFM_REALM(calloc(
	    1, sizeof(SFMRealmStruct)
	));
	if(realm == NULL)
	    return(NULL);

	/* Reset values */
	realm->lapsed_time = 0l;
	realm->time_compensation = 1.0;
	realm->time_compression = 1.0;

	realm->gravity = SAR_GRAVITY;

	realm->airborne_cb_client_data = NULL;
	realm->airborne_cb = NULL;

	realm->touch_down_cb_client_data = NULL;
	realm->touch_down_cb = NULL;

	realm->parked_cb_client_data = NULL;
	realm->parked_cb = NULL;

	realm->collision_cb_client_data = NULL;
	realm->collision_cb = NULL;

	realm->model = NULL;
	realm->total_models = 0;

	return(realm);
}

/*
 *	Deletes the FDM Realm.
 */
void SFMShutdown(SFMRealmStruct *realm)
{
	if(realm == NULL)
	    return;

	/* Delete all FDMs */
	while(realm->total_models > 0)
	{
	    if(realm->model[0] == NULL)
		break;

	    /* Delete this SFMModel, this will also call the
	     * destroy_model_cb on the Realm if it is set.
	     *
	     * Note that each call to this function will
	     * deincrement realm->total_models by one
	     */
	    SFMModelDelete(realm, realm->model[0]);
	}

	free(realm);
}

/*
 *	Sets the FDM Realm's timing.
 *
 *	Future calls to SFM*() functions will use the timing values
 *	set here.
 */
void SFMSetTiming(SFMRealmStruct *realm, SFMTime lapsed_ms)
{
	if(realm == NULL)
	    return;

	if(lapsed_ms < 0l)
	    lapsed_ms = 0l;

	/* Update timings */
	realm->lapsed_time = lapsed_ms;
	realm->time_compensation = (double)(
	    (double)lapsed_ms / (double)SFMCycleUnitsMS
	);
}

/*
 *	Sets the FDM Realm's time compression.
 */
void SFMSetTimeCompression(SFMRealmStruct *realm, double compression)
{
	if(realm == NULL)
	    return;

	if(compression < 0.0)
	    compression = 0.0;

	realm->time_compression = compression;
}

/*
 *	Updates the FDM Realm's FDMs and timing.
 */
void SFMUpdateRealm(SFMRealmStruct *realm, SFMTime lapsed_ms)
{
	int m_num;
	SFMModelStruct *m;

	if(realm == NULL)
	    return;

	SFMSetTiming(realm, lapsed_ms);

	/* Update each FDM */
	for(m_num = 0; m_num < realm->total_models; m_num++)
	{
	    m = realm->model[m_num];
	    if(m == NULL)
		continue;

/* TODO */

	}
}
