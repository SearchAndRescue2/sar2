#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/stat.h>
#ifdef __MSW__
# include <windows.h>
#endif
#include "../include/string.h"
#include "../include/disk.h"
#include "../include/fio.h"
#include <GL/gl.h>
#include "gw.h"
#include "v3dtex.h"
#include "cpvalues.h"
#include "cpins.h"
#include "cpinsaltimeter.h"
#include "cpinsbearing.h"
#include "cpinshorizon.h"
#include "cp.h"
#include "cpfio.h"
#include "config.h"


static int CPLoadInstrumentAltimeter(
        FILE *fp, CPIns *ins, const char *parm
);
static int CPLoadInstrumentBearing(
        FILE *fp, CPIns *ins, const char *parm
);
static int CPLoadInstrumentHorizon(
        FILE *fp, CPIns *ins, const char *parm
);
static int CPLoadInstrumentFromFile(
        ControlPanel *cp, const char *path,
	CPIns *ins, int ins_num,
        const char *filename
);
int CPLoadFromFile(ControlPanel *cp, const char *path);


#ifndef PI
# define PI     3.14159265
#endif

#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))

#define RADTODEG(r)     ((r) * 180.0 / PI)
#define DEGTORAD(d)     ((d) * PI / 180.0)


/*
 *	Altimeter instrument specific loading, called by
 *      CPLoadInstrumentFromFile().
 */
static int CPLoadInstrumentAltimeter(
        FILE *fp, CPIns *ins, const char *parm
)
{
        /* CPInsAltimeter *insv = CPINS_ALTIMETER(ins); */
        int handled_parm = 0;

        return(handled_parm);
}

/*
 *	Bearing instrument specific loading, called by
 *	CPLoadInstrumentFromFile().
 */
static int CPLoadInstrumentBearing(
	FILE *fp, CPIns *ins, const char *parm
)
{
	CPInsBearing *insv = CPINS_BEARING(ins);
	int handled_parm = 0;

	/* BearingMagneticVelocityDecceleration */
	if(!strcasecmp(parm, "BearingMagneticVelocityDecceleration") ||
           !strcasecmp(parm, "BearingMagneticVelocityDeccel")
	)
	{
            double vf[1];
            FGetValuesF(fp, vf, 1);
	    insv->bearing_mag_vel_dec = (float)DEGTORAD(
		MAX(vf[0], 1.0)
	    );
	    handled_parm++;
	}

	return(handled_parm);
}

/*
 *      Horizon instrument specific loading, called by
 *      CPLoadInstrumentFromFile().
 */
static int CPLoadInstrumentHorizon(
        FILE *fp, CPIns *ins, const char *parm
)
{
        CPInsHorizon *insv = CPINS_HORIZON(ins);
        int handled_parm = 0;

        /* DegreesVisible */
        if(!strcasecmp(parm, "DegreesVisible"))
        {
            double vf[1];
            FGetValuesF(fp, vf, 1);
            insv->deg_visible = (int)MAX(vf[0], 1.0);
            handled_parm++;
        }
        /* BankNotchRadius */
        else if(!strcasecmp(parm, "BankNotchRadius"))
        {
            double vf[1];
            FGetValuesF(fp, vf, 1);
            insv->bank_notch_radius = (float)vf[0];
            handled_parm++;
        }

        return(handled_parm);
}

/*
 *	Called by CPLoadFromFile().
 *
 *	Loads an instrument configuration file that will set up values
 *	specific to that instrument.
 *
 *	Returns non-zero on error.
 */
static int CPLoadInstrumentFromFile(
	ControlPanel *cp, const char *path,
	CPIns *ins, int ins_num,
	const char *filename
)
{
	int handled_parm;
        FILE            *fp;
        char            *buf = NULL;
        struct stat     stat_buf;


        if((cp == NULL) || (filename == NULL))
            return(-1);

        if(*filename == '\0')
            return(-1);

	/* No instrument in context? */
	if(ins == NULL)
	{
            fprintf(stderr, "%s: No instrument in context.\n", filename);
            return(-1);
        }

	/* Check if file exists. */
        if(stat(filename, &stat_buf))
        {
            fprintf(stderr, "%s: No such file.\n", filename);
            return(-1);
        }

        /* Open instrument configuration file. */
        fp = FOpen(filename, "rb");
        if(fp == NULL)
        {
            fprintf(stderr, "%s: Cannot open.\n", filename);
            return(-1);
        }

        /* Begin reading instruent configuration file. */
        do
        {
            buf = FSeekNextParm(
                fp,
                buf,
                SAR_COMMENT_CHAR,
                SAR_CFG_DELIM_CHAR
            );
	    if(buf == NULL)
		break;

	    handled_parm = 0;

            /* Version */
            if(!strcasecmp(buf, "Version"))
            {
                double vf[2];
                FGetValuesF(fp, vf, 2);

                /* Ignore. */
		handled_parm++;
            }

	    /* Resolution */
	    else if(!strcasecmp(buf, "Resolution"))
            {
		int res_width, res_height;
                double vf[2];
                FGetValuesF(fp, vf, 2);
		res_width = (int)vf[0];
		res_height = (int)vf[1];
		if(res_width < 2)
		{
		    fprintf(
			stderr,
"%s: Resolution: Error: Width %i cannot be less than 2 (must be a 2^x value).\n",
			filename, res_width
		    );
		    res_width = 2;
		}
                if(res_height < 2)
                {
                    fprintf(
                        stderr,
"%s: Resolution: Error: Height %i cannot be less than 2 (must be a 2^x value).\n",
                        filename, res_height
                    );
                    res_width = 2;
                }
		CPInsSetResolution(ins, res_width, res_height);
		handled_parm++;
	    }

            /* TextureBackground */
            else if(!strcasecmp(buf, "TextureBackground") ||
                    !strcasecmp(buf, "TextureBG")
	    )
            {
                char *s = FGetString(fp);
                if(!ISPATHABSOLUTE(s))
                {
                    char *s2 = STRDUP(PrefixPaths(path, s));
                    free(s);
                    s = s2;
                }
		CPInsSetTextureBG(ins, s);
                free(s);
                handled_parm++;
	    }

            /* TextureForeground */
            else if(!strcasecmp(buf, "TextureForeground") ||
                    !strcasecmp(buf, "TextureFG")
            )
            {
                char *s = FGetString(fp);
                if(!ISPATHABSOLUTE(s))
                {
                    char *s2 = STRDUP(PrefixPaths(path, s));
                    free(s);
                    s = s2;
                }
                CPInsSetTextureFG(ins, s);
                free(s);
                handled_parm++;
            }

            /* ColorBackgroundNormal */
            else if(!strcasecmp(buf, "ColorBackgroundNormal") ||
                    !strcasecmp(buf, "ColorBGNormal")
            )
            {
		CPColor *c = &ins->color_bg[CP_COLOR_STATE_NORMAL];
                double vf[3];
                FGetValuesF(fp, vf, 3);
		c->r = (GLfloat)CLIP(vf[0], 0.0, 1.0);
                c->g = (GLfloat)CLIP(vf[1], 0.0, 1.0);
                c->b = (GLfloat)CLIP(vf[2], 0.0, 1.0);
                handled_parm++;
            }
            /* ColorBackgroundDim */
            else if(!strcasecmp(buf, "ColorBackgroundDim") ||
                    !strcasecmp(buf, "ColorBGDim")
            )
            {
                CPColor *c = &ins->color_bg[CP_COLOR_STATE_DIM];
                double vf[3];
                FGetValuesF(fp, vf, 3);
                c->r = (GLfloat)CLIP(vf[0], 0.0, 1.0);
                c->g = (GLfloat)CLIP(vf[1], 0.0, 1.0);
                c->b = (GLfloat)CLIP(vf[2], 0.0, 1.0);
                handled_parm++;
            }
            /* ColorBackgroundDark */
            else if(!strcasecmp(buf, "ColorBackgroundDark") ||
                    !strcasecmp(buf, "ColorBGDark")
            )
            {
                CPColor *c = &ins->color_bg[CP_COLOR_STATE_DARK];
                double vf[3];
                FGetValuesF(fp, vf, 3);
                c->r = (GLfloat)CLIP(vf[0], 0.0, 1.0);
                c->g = (GLfloat)CLIP(vf[1], 0.0, 1.0);
                c->b = (GLfloat)CLIP(vf[2], 0.0, 1.0);
                handled_parm++;
            }
            /* ColorBackgroundLighted */
            else if(!strcasecmp(buf, "ColorBackgroundLighted") ||
                    !strcasecmp(buf, "ColorBGLighted")
            )
            {
                CPColor *c = &ins->color_bg[CP_COLOR_STATE_LIGHTED];
                double vf[3];
                FGetValuesF(fp, vf, 3);
                c->r = (GLfloat)CLIP(vf[0], 0.0, 1.0);
                c->g = (GLfloat)CLIP(vf[1], 0.0, 1.0);
                c->b = (GLfloat)CLIP(vf[2], 0.0, 1.0);
                handled_parm++;
            }

            /* ColorForegroundNormal */
            else if(!strcasecmp(buf, "ColorForegroundNormal") ||
                    !strcasecmp(buf, "ColorFGNormal")
            )
            {
                CPColor *c = &ins->color_fg[CP_COLOR_STATE_NORMAL];
                double vf[3];
                FGetValuesF(fp, vf, 3);
                c->r = (GLfloat)CLIP(vf[0], 0.0, 1.0);
                c->g = (GLfloat)CLIP(vf[1], 0.0, 1.0);
                c->b = (GLfloat)CLIP(vf[2], 0.0, 1.0);
                handled_parm++;
            }
            /* ColorForegroundDim */
            else if(!strcasecmp(buf, "ColorForegroundDim") ||
                    !strcasecmp(buf, "ColorFGDim")
            )
            {
                CPColor *c = &ins->color_fg[CP_COLOR_STATE_DIM];
                double vf[3];
                FGetValuesF(fp, vf, 3);
                c->r = (GLfloat)CLIP(vf[0], 0.0, 1.0);
                c->g = (GLfloat)CLIP(vf[1], 0.0, 1.0);
                c->b = (GLfloat)CLIP(vf[2], 0.0, 1.0);
                handled_parm++;
            }
            /* ColorForegroundDark */
            else if(!strcasecmp(buf, "ColorForegroundDark") ||
                    !strcasecmp(buf, "ColorFGDark")
            )
            {
                CPColor *c = &ins->color_fg[CP_COLOR_STATE_DARK];
                double vf[3];
                FGetValuesF(fp, vf, 3);
                c->r = (GLfloat)CLIP(vf[0], 0.0, 1.0);
                c->g = (GLfloat)CLIP(vf[1], 0.0, 1.0);
                c->b = (GLfloat)CLIP(vf[2], 0.0, 1.0);
                handled_parm++;
            }
            /* ColorForegroundLighted */
            else if(!strcasecmp(buf, "ColorForegroundLighted") ||
                    !strcasecmp(buf, "ColorFGLighted")
            )
            {
                CPColor *c = &ins->color_fg[CP_COLOR_STATE_LIGHTED];
                double vf[3];
                FGetValuesF(fp, vf, 3);
                c->r = (GLfloat)CLIP(vf[0], 0.0, 1.0);
                c->g = (GLfloat)CLIP(vf[1], 0.0, 1.0);
                c->b = (GLfloat)CLIP(vf[2], 0.0, 1.0);
                handled_parm++;
            }

            /* ColorTextNormal */
            else if(!strcasecmp(buf, "ColorTextNormal"))
            {
                CPColor *c = &ins->color_text[CP_COLOR_STATE_NORMAL];
                double vf[3];
                FGetValuesF(fp, vf, 3);
                c->r = (GLfloat)CLIP(vf[0], 0.0, 1.0);
                c->g = (GLfloat)CLIP(vf[1], 0.0, 1.0);
                c->b = (GLfloat)CLIP(vf[2], 0.0, 1.0);
                handled_parm++;
            }
            /* ColorTextDim */
            else if(!strcasecmp(buf, "ColorTextDim"))
            {
                CPColor *c = &ins->color_text[CP_COLOR_STATE_DIM];
                double vf[3];
                FGetValuesF(fp, vf, 3);
                c->r = (GLfloat)CLIP(vf[0], 0.0, 1.0);
                c->g = (GLfloat)CLIP(vf[1], 0.0, 1.0);
                c->b = (GLfloat)CLIP(vf[2], 0.0, 1.0);
                handled_parm++;
            }
            /* ColorTextDark */
            else if(!strcasecmp(buf, "ColorTextDark"))
            {
                CPColor *c = &ins->color_text[CP_COLOR_STATE_DARK];
                double vf[3];
                FGetValuesF(fp, vf, 3);
                c->r = (GLfloat)CLIP(vf[0], 0.0, 1.0);
                c->g = (GLfloat)CLIP(vf[1], 0.0, 1.0);
                c->b = (GLfloat)CLIP(vf[2], 0.0, 1.0);
                handled_parm++;
            }
            /* ColorTextLighted */
            else if(!strcasecmp(buf, "ColorTextLighted"))
            {
                CPColor *c = &ins->color_text[CP_COLOR_STATE_LIGHTED];
                double vf[3];
                FGetValuesF(fp, vf, 3);
                c->r = (GLfloat)CLIP(vf[0], 0.0, 1.0);
                c->g = (GLfloat)CLIP(vf[1], 0.0, 1.0);
                c->b = (GLfloat)CLIP(vf[2], 0.0, 1.0);
                handled_parm++;
            }

	    /* If the parameter was not handled then pass it to an
	     * instrument specific handler.
	     */
	    if(handled_parm <= 0)
	    {
		/* Call instrument specific loading function to
		 * deal with this parameter.
		 */
		switch(CPINS_TYPE(ins))
		{
                  case CPINS_TYPE_ALTIMETER:
                    handled_parm += CPLoadInstrumentAltimeter(
                        fp, ins, buf
                    );
                    break;
                  case CPINS_TYPE_BEARING:
                    handled_parm += CPLoadInstrumentBearing(
                        fp, ins, buf
                    );
                    break;
		  case CPINS_TYPE_HORIZON:
		    handled_parm += CPLoadInstrumentHorizon(
			fp, ins, buf
		    );
		    break;
		}
		/* Parameter still not handled? */
		if(handled_parm <= 0)
		{
		    fprintf(
			stderr,
			"%s: Unsupported parameter `%s'.\n",
			filename, buf
		    );
		    FSeekNextLine(fp);
		}
            }

        } while(1);

        /* Close instrument configuration file. */
        FClose(fp);

        return(0);
}


/*
 *	Loads control panel from file.
 *
 *	The given path must be a prefix to the control panel directory
 *	(not an actual file).  The appropriate files will be loaded
 *	in the directory specified by path.
 *
 *	Returns non-zero on error.
 */
int CPLoadFromFile(ControlPanel *cp, const char *path)
{
        FILE		*fp;
        char		*buf = NULL;

	char		*cp_file,
			*cp_tex_file;

	struct stat	stat_buf;

	int		ins_num = -1;
	CPIns		*ins = NULL;
	

	if((cp == NULL) || (path == NULL))
	    return(-1);

	if(*path == '\0')
	    return(-1);

        if(stat(path, &stat_buf))
	{
	    fprintf(
		stderr,
		"%s: No such directory.\n",
		path
	    );
	    return(-1);
	}

	/* Get file names. */
	/* Control panel configuration file. */
	cp_file = STRDUP(
	    PrefixPaths(path, SAR_DEF_INSTRUMENTS_FILE)
	);
	/* Control panel texture. */
        cp_tex_file = STRDUP(
            PrefixPaths(path, SAR_DEF_CONTROL_PANEL_TEX_FILE)
        );

#define DO_FREE_LOCALS	{	\
 free(cp_file);			\
 free(cp_tex_file);		\
}

	/* Check if required files exist. */
	if(stat(cp_file,  &stat_buf))
        {
            fprintf(stderr, "%s: No such file.\n", cp_file);
	    DO_FREE_LOCALS
            return(-1);
        }

	/* Open control panel configuration file. */
        fp = FOpen(cp_file, "rb");
        if(fp == NULL)
        {
            fprintf(stderr, "%s: Cannot open.\n", cp_file);
	    DO_FREE_LOCALS
            return(-1);
        }

	/* Begin reading control panel configuration file. */
        do
        {
            buf = FSeekNextParm(
                fp,
                buf,
                SAR_COMMENT_CHAR,
                SAR_CFG_DELIM_CHAR
            );
            if(buf == NULL)
                break;

	    /* Begin handling parameter. */

	    /* Version */
            if(!strcasecmp(buf, "Version"))
            {
		double vf[2];
                FGetValuesF(fp, vf, 2);

		/* Ignore. */
            }
	    /* InstrumentNew */
	    else if(!strcasecmp(buf, "InstrumentNew"))
            {
                char *type = FGetString(fp);
		/* Create new instrument by the given type. */
		if(!strcasecmp(type, "Instrument"))
		{
		    ins = CPInsNew(sizeof(CPIns), cp, CPINS_TYPE_INS, type);
		}
                /* Altimeter */
                else if(!strcasecmp(type, "Altimeter"))
                {
                    ins = CPInsAltimeterNew(cp);
                }
		/* Bearing */
                else if(!strcasecmp(type, "Bearing"))
                {
                    ins = CPInsBearingNew(cp);
                }
		/* Artificial Horizon */
                else if(!strcasecmp(type, "ArtificialHorizon") ||
		        !strcasecmp(type, "Horizon")
		)
                {
                    ins = CPInsHorizonNew(cp);
                }
		else
		{
		    fprintf(
			stderr,
"%s: InstrumentNew: Unsupported instrument type \"%s\".\n",
			cp_file, type
		    );
		    ins = NULL;
		    ins_num = -1;
		}
		/* Created new instrument successfully? */
		if(ins != NULL)
		{
		    ins_num = CPAppendIns(cp, ins);
		    if(ins_num < 0)
			fprintf(
			    stderr,
"%s: InstrumentNew: Error appending instrument type \"%s\" to control panel.\n",
			    cp_file, type
			);
		}
		free(type);
	    }
            /* Position */
            else if(!strcasecmp(buf, "Position"))
            {
		double vf[2];
                FGetValuesF(fp, vf, 2);
		/* Set instrument position coefficient relative to the
		 * control panel's upper left corner.
		 */
		CPInsSetPosition(
		    ins,
		    (GLfloat)vf[0],
                    (GLfloat)vf[1]
		);
            }
            /* Size */
            else if(!strcasecmp(buf, "Size"))
            {
                double vf[2];
                FGetValuesF(fp, vf, 2);
                /* Set instrument size coefficient. */
                CPInsSetSize(
                    ins,
                    (GLfloat)vf[0],
                    (GLfloat)vf[1]
                );
            }
            /* ConfigurationFile */
            else if(!strcasecmp(buf, "ConfigurationFile"))
            {
                char *ins_file = FGetString(fp);
		if(!ISPATHABSOLUTE(ins_file))
		{
		    char *s = STRDUP(
			PrefixPaths(path, ins_file)
		    );
		    free(ins_file);
		    ins_file = s;
		}
		CPLoadInstrumentFromFile(
		    cp, path, ins, ins_num, ins_file
		);
		free(ins_file);
            }

            else
            {
                fprintf(
		    stderr,
                    "%s: Unsupported parameter `%s'.\n",
                    cp_file, buf
                );
                FSeekNextLine(fp);
            }

        } while(1);

	/* Close control panel configuration file. */
        FClose(fp);

	/* Load control panel texture. */
	CPSetTexture(cp, cp_tex_file);

	DO_FREE_LOCALS
#undef DO_FREE_LOCALS

	return(0);
}
