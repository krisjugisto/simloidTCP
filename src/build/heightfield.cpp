
#include <build/heightfield.h>
#include <basic/common.h>

dReal heightfield_callback(void* pUserData, int x, int z);

Heightfield::Heightfield(const dSpaceID &space, const std::string name, const Vector3 pos, const Color4 color)
: name(name)
, color(color)
{
    dsPrint("Creating heightfield landscape '%s'...", name.c_str());

    heightid = dGeomHeightfieldDataCreate();

    /* create heightfield */
    dGeomHeightfieldDataBuildCallback(heightid, NULL, heightfield_callback,
        HFIELD_WIDTH, HFIELD_DEPTH, HFIELD_WSTEP, HFIELD_DSTEP,
        REAL(1.0), REAL(0.0), REAL(0.0), 0);

    // Give some very bounds which, while conservative,
    // makes AABB computation more accurate than +/-INF.
    dGeomHeightfieldDataSetBounds(heightid, REAL(-4.0), REAL(+6.0));
    geometry = dCreateHeightfield(space, heightid, 1);

    // Rotate so Z is up, not Y (which is the default orientation)
    dMatrix3 R;
    dRSetIdentity(R);
    dRFromAxisAndAngle(R, 1, 0, 0, common::deg2rad(90));

    // Place it.
    dGeomSetRotation(geometry, R);
    dGeomSetPosition(geometry, pos.x, pos.y, pos.z);

    dsPrint("done.\n");

}

Heightfield::~Heightfield()
{
    dsPrint("Destroying heightfield %s...", name.c_str());
    dGeomHeightfieldDataDestroy(heightid);
    dsPrint("done.\n");
}

dReal
heightfield_callback(void* /*pUserData*/, int x, int y)
{
    dReal fx = (((dReal) x * 2.0) - (HFIELD_WSTEP - 1)) / (dReal)(HFIELD_WSTEP - 1);
    dReal fy = (((dReal) y * 2.0) - (HFIELD_DSTEP - 1)) / (dReal)(HFIELD_DSTEP - 1);


    dReal h;
    //h = REAL(1.0) + tanh(5*fz);
    dReal Y = 0.5 * tanh(4.0 * cos(-constants::m_pi * fx)) + 0.5;

    if (fy != 2.0) //TODO ment to be -2 ?
        h = Y*(fy + 1) * pow(cos(4.5 * constants::m_pi * (1.0 / (fy + 2))), 2);
    else
        h = 0.0;

    h += 0.0001;
    return h;
}


void
Heightfield::draw(void)
{
    const dReal* pReal = dGeomGetPosition(geometry);
    const dReal* RReal = dGeomGetRotation(geometry);

    // Set ox and oz to zero for DHEIGHTFIELD_CORNER_ORIGIN mode.
    int ox = (int) (-HFIELD_WIDTH/2);
    int oz = (int) (-HFIELD_DEPTH/2);

    dsSetColorAlpha(color.r, color.g, color.b, color.a);

    for (unsigned int i = 0; i < HFIELD_WSTEP - 1; ++i)
        for (unsigned int j = 0; j < HFIELD_DSTEP - 1; ++j)
        {
            dReal a[3], b[3], c[3], d[3];

            a[0] = ox + ( i ) * HFIELD_WSAMP;
            a[1] = heightfield_callback( NULL, i, j );
            a[2] = oz + ( j ) * HFIELD_DSAMP;

            b[0] = ox + ( i + 1 ) * HFIELD_WSAMP;
            b[1] = heightfield_callback( NULL, i + 1, j );
            b[2] = oz + ( j ) * HFIELD_DSAMP;

            c[0] = ox + ( i ) * HFIELD_WSAMP;
            c[1] = heightfield_callback( NULL, i, j + 1 );
            c[2] = oz + ( j + 1 ) * HFIELD_DSAMP;

            d[0] = ox + ( i + 1 ) * HFIELD_WSAMP;
            d[1] = heightfield_callback( NULL, i + 1, j + 1 );
            d[2] = oz + ( j + 1 ) * HFIELD_DSAMP;

            dsDrawTriangleD(pReal, RReal, a, c, b, 1);
            dsDrawTriangleD(pReal, RReal, b, c, d, 1);
        }

}
