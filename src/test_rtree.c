/*
** 2010 August 28
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Code for testing all sorts of SQLite interfaces. This code
** is not included in the SQLite library. 
*/

#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

/* Solely for the UNUSED_PARAMETER() macro. */
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_RTREE
/* 
** Type used to cache parameter information for the "circle" r-tree geometry
** callback.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Circle Circle;
ರಚನೆ Circle {
  ರಚನೆ Box {
    ದ್ವಿಗುಣ xmin;
    ದ್ವಿಗುಣ xmax;
    ದ್ವಿಗುಣ ymin;
    ದ್ವಿಗುಣ ymax;
  } aBox[2];
  ದ್ವಿಗುಣ centerx;
  ದ್ವಿಗುಣ centery;
  ದ್ವಿಗುಣ radius;
  ದ್ವಿಗುಣ mxArea;
  ಪೂರ್ಣಾಂಕ eScoreType;
};

/*
** Destructor function for Circle objects allocated by circle_geom().
*/
ಸ್ಥಿರ ಶೂನ್ಯ circle_del(ಶೂನ್ಯ *p){
  sqlite3_free(p);
}

/*
** Implementation of "circle" r-tree geometry callback.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ circle_geom(
  sqlite3_rtree_geometry *p,
  ಪೂರ್ಣಾಂಕ nCoord, 
  sqlite3_rtree_dbl *aCoord,
  ಪೂರ್ಣಾಂಕ *pRes
){
  ಪೂರ್ಣಾಂಕ i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  ದ್ವಿಗುಣ xmin, xmax;              /* X dimensions of box being tested */
  ದ್ವಿಗುಣ ymin, ymax;              /* X dimensions of box being tested */

  xmin = aCoord[0];
  xmax = aCoord[1];
  ymin = aCoord[2];
  ymax = aCoord[3];
  pCircle = (Circle *)p->pUser;
  ಆದರೆ( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    ಆದರೆ( nCoord!=4 ) ಹಿಂತಿರುಗಿ SQLITE_ERROR;

    /* Test that the correct number of parameters (3) have been supplied,
    ** and that the parameters are in range (that the radius of the circle 
    ** radius is greater than zero). */
    ಆದರೆ( p->nParam!=3 || p->aParam[2]<0.0 ) ಹಿಂತಿರುಗಿ SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(ಗಾತ್ರ(Circle)));
    ಆದರೆ( !pCircle ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    p->xDelUser = circle_del;

    /* Record the center and radius of the circular region. One way that
    ** tested bounding boxes that intersect the circular region are detected
    ** is by testing if each corner of the bounding box lies within radius
    ** units of the center of the circle. */
    pCircle->centerx = p->aParam[0];
    pCircle->centery = p->aParam[1];
    pCircle->radius = p->aParam[2];

    /* Define two bounding box regions. The first, aBox[0], extends to
    ** infinity in the X dimension. It covers the same range of the Y dimension
    ** as the circular region. The second, aBox[1], extends to infinity in
    ** the Y dimension and is constrained to the range of the circle in the
    ** X dimension.
    **
    ** Then imagine each box is split in half along its short axis by a line
    ** that intersects the center of the circular region. A bounding box
    ** being tested can be said to intersect the circular region if it contains
    ** points from each half of either of the two infinite bounding boxes.
    */
    pCircle->aBox[0].xmin = pCircle->centerx;
    pCircle->aBox[0].xmax = pCircle->centerx;
    pCircle->aBox[0].ymin = pCircle->centery + pCircle->radius;
    pCircle->aBox[0].ymax = pCircle->centery - pCircle->radius;
    pCircle->aBox[1].xmin = pCircle->centerx + pCircle->radius;
    pCircle->aBox[1].xmax = pCircle->centerx - pCircle->radius;
    pCircle->aBox[1].ymin = pCircle->centery;
    pCircle->aBox[1].ymax = pCircle->centery;
    pCircle->mxArea = (xmax - xmin)*(ymax - ymin) + 1.0;
  }

  /* Check if any of the 4 corners of the bounding-box being tested lie 
  ** inside the circular region. If they do, then the bounding-box does
  ** intersect the region of interest. Set the output variable to true and
  ** return SQLITE_OK in this case. */
  ಗಾಗಿ(i=0; i<4; i++){
    ದ್ವಿಗುಣ x = (i&0x01) ? xmax : xmin;
    ದ್ವಿಗುಣ y = (i&0x02) ? ymax : ymin;
    ದ್ವಿಗುಣ d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    ಆದರೆ( d2<(pCircle->radius*pCircle->radius) ){
      *pRes = 1;
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  ಗಾಗಿ(i=0; i<2; i++){
    ಆದರೆ( xmin<=pCircle->aBox[i].xmin 
     && xmax>=pCircle->aBox[i].xmax 
     && ymin<=pCircle->aBox[i].ymin 
     && ymax>=pCircle->aBox[i].ymax 
    ){
      *pRes = 1;
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
  }

  /* The specified bounding box does not intersect the circular region. Set
  ** the output variable to zero and return SQLITE_OK. */
  *pRes = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Implementation of "circle" r-tree geometry callback using the 
** 2nd-generation interface that allows scoring.
**
** Two calling forms:
**
**          Qcircle(X,Y,Radius,eType)        -- All values are doubles
**          Qcircle('x:X y:Y r:R e:ETYPE')   -- Single string parameter
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ circle_query_func(sqlite3_rtree_query_info *p){
  ಪೂರ್ಣಾಂಕ i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  ದ್ವಿಗುಣ xmin, xmax;              /* X dimensions of box being tested */
  ದ್ವಿಗುಣ ymin, ymax;              /* X dimensions of box being tested */
  ಪೂರ್ಣಾಂಕ nWithin = 0;                /* Number of corners inside the circle */

  xmin = p->aCoord[0];
  xmax = p->aCoord[1];
  ymin = p->aCoord[2];
  ymax = p->aCoord[3];
  pCircle = (Circle *)p->pUser;
  ಆದರೆ( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    ಆದರೆ( p->nCoord!=4 ) ಹಿಂತಿರುಗಿ SQLITE_ERROR;

    /* Test that the correct number of parameters (1 or 4) have been supplied.
    */
    ಆದರೆ( p->nParam!=4 && p->nParam!=1 ) ಹಿಂತಿರುಗಿ SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(ಗಾತ್ರ(Circle)));
    ಆದರೆ( !pCircle ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    p->xDelUser = circle_del;

    /* Record the center and radius of the circular region. One way that
    ** tested bounding boxes that intersect the circular region are detected
    ** is by testing if each corner of the bounding box lies within radius
    ** units of the center of the circle. */
    ಆದರೆ( p->nParam==4 ){
      pCircle->centerx = p->aParam[0];
      pCircle->centery = p->aParam[1];
      pCircle->radius = p->aParam[2];
      pCircle->eScoreType = (ಪೂರ್ಣಾಂಕ)p->aParam[3];
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(p->apSqlParam[0]);
      pCircle->centerx = 0.0;
      pCircle->centery = 0.0;
      pCircle->radius = 0.0;
      pCircle->eScoreType = 0;
      ಆಗ( z && z[0] ){
        ಆದರೆ( z[0]=='r' && z[1]==':' ){
          pCircle->radius = atof(&z[2]);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[0]=='x' && z[1]==':' ){
          pCircle->centerx = atof(&z[2]);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[0]=='y' && z[1]==':' ){
          pCircle->centery = atof(&z[2]);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[0]=='e' && z[1]==':' ){
          pCircle->eScoreType = (ಪೂರ್ಣಾಂಕ)atof(&z[2]);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[0]==' ' ){
          z++;
          ಮುಂದುವರಿ;
        }
        ಆಗ( z[0]!=0 && z[0]!=' ' ) z++;
        ಆಗ( z[0]==' ' ) z++;
      }
    }
    ಆದರೆ( pCircle->radius<0.0 ){
      sqlite3_free(pCircle);
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    }

    /* Define two bounding box regions. The first, aBox[0], extends to
    ** infinity in the X dimension. It covers the same range of the Y dimension
    ** as the circular region. The second, aBox[1], extends to infinity in
    ** the Y dimension and is constrained to the range of the circle in the
    ** X dimension.
    **
    ** Then imagine each box is split in half along its short axis by a line
    ** that intersects the center of the circular region. A bounding box
    ** being tested can be said to intersect the circular region if it contains
    ** points from each half of either of the two infinite bounding boxes.
    */
    pCircle->aBox[0].xmin = pCircle->centerx;
    pCircle->aBox[0].xmax = pCircle->centerx;
    pCircle->aBox[0].ymin = pCircle->centery + pCircle->radius;
    pCircle->aBox[0].ymax = pCircle->centery - pCircle->radius;
    pCircle->aBox[1].xmin = pCircle->centerx + pCircle->radius;
    pCircle->aBox[1].xmax = pCircle->centerx - pCircle->radius;
    pCircle->aBox[1].ymin = pCircle->centery;
    pCircle->aBox[1].ymax = pCircle->centery;
    pCircle->mxArea = 200.0*200.0;
  }

  /* Check if any of the 4 corners of the bounding-box being tested lie 
  ** inside the circular region. If they do, then the bounding-box does
  ** intersect the region of interest. Set the output variable to true and
  ** return SQLITE_OK in this case. */
  ಗಾಗಿ(i=0; i<4; i++){
    ದ್ವಿಗುಣ x = (i&0x01) ? xmax : xmin;
    ದ್ವಿಗುಣ y = (i&0x02) ? ymax : ymin;
    ದ್ವಿಗುಣ d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    ಆದರೆ( d2<(pCircle->radius*pCircle->radius) ) nWithin++;
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  ಆದರೆ( nWithin==0 ){
    ಗಾಗಿ(i=0; i<2; i++){
      ಆದರೆ( xmin<=pCircle->aBox[i].xmin 
       && xmax>=pCircle->aBox[i].xmax 
       && ymin<=pCircle->aBox[i].ymin 
       && ymax>=pCircle->aBox[i].ymax 
      ){
        nWithin = 1;
        ನಿಲ್ಲಿಸು;
      }
    }
  }

  ಆದರೆ( pCircle->eScoreType==1 ){
    /* Depth first search */
    p->rScore = p->iLevel;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pCircle->eScoreType==2 ){
    /* Breadth first search */
    p->rScore = 100 - p->iLevel;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pCircle->eScoreType==3 ){
    /* Depth-first search, except sort the leaf nodes by area with
    ** the largest area first */
    ಆದರೆ( p->iLevel==1 ){
      p->rScore = 1.0 - (xmax-xmin)*(ymax-ymin)/pCircle->mxArea;
      ಆದರೆ( p->rScore<0.01 ) p->rScore = 0.01;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p->rScore = 0.0;
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pCircle->eScoreType==4 ){
    /* Depth-first search, except exclude odd rowids */
    p->rScore = p->iLevel;
    ಆದರೆ( p->iRowid&1 ) nWithin = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Breadth-first search, except exclude odd rowids */
    p->rScore = 100 - p->iLevel;
    ಆದರೆ( p->iRowid&1 ) nWithin = 0;
  }
  ಆದರೆ( nWithin==0 ){
    p->eWithin = NOT_WITHIN;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nWithin>=4 ){
    p->eWithin = FULLY_WITHIN;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->eWithin = PARTLY_WITHIN;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
/*
** Implementation of "breadthfirstsearch" r-tree geometry callback using the 
** 2nd-generation interface that allows scoring.
**
**     ... WHERE id MATCH breadthfirstsearch($x0,$x1,$y0,$y1) ...
**
** It returns all entries whose bounding boxes overlap with $x0,$x1,$y0,$y1.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bfs_query_func(sqlite3_rtree_query_info *p){
  ದ್ವಿಗುಣ x0,x1,y0,y1;        /* Dimensions of box being tested */
  ದ್ವಿಗುಣ bx0,bx1,by0,by1;    /* Boundary of the query function */

  ಆದರೆ( p->nParam!=4 ) ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  x0 = p->aCoord[0];
  x1 = p->aCoord[1];
  y0 = p->aCoord[2];
  y1 = p->aCoord[3];
  bx0 = p->aParam[0];
  bx1 = p->aParam[1];
  by0 = p->aParam[2];
  by1 = p->aParam[3];
  p->rScore = 100 - p->iLevel;
  ಆದರೆ( p->e_ಪೋಷಕ_Within==FULLY_WITHIN ){
    p->eWithin = FULLY_WITHIN;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( x0>=bx0 && x1<=bx1 && y0>=by0 && y1<=by1 ){
    p->eWithin = FULLY_WITHIN;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( x1>=bx0 && x0<=bx1 && y1>=by0 && y0<=by1 ){
    p->eWithin = PARTLY_WITHIN;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->eWithin = NOT_WITHIN;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* END of implementation of "circle" geometry callback.
**************************************************************************
*************************************************************************/

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Cube Cube;
ರಚನೆ Cube {
  ದ್ವಿಗುಣ x;
  ದ್ವಿಗುಣ y;
  ದ್ವಿಗುಣ z;
  ದ್ವಿಗುಣ ಅಗಲ;
  ದ್ವಿಗುಣ ಎತ್ತರ;
  ದ್ವಿಗುಣ ಆಳ;
};

ಸ್ಥಿರ ಶೂನ್ಯ cube_context_free(ಶೂನ್ಯ *p){
  sqlite3_free(p);
}

/*
** The context pointer registered along with the 'cube' callback is
** always ((void *)&gHere). This is just to facilitate testing, it is not
** actually used for anything.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ g_ಇಲ್ಲಿ = 42;

/*
** Implementation of a simple r-tree geom callback to test for intersection
** of r-tree rows with a "cube" shape. Cubes are defined by six scalar
** coordinates as follows:
**
**   cube(x, y, z, width, height, depth)
**
** The width, height and depth parameters must all be greater than zero.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cube_geom(
  sqlite3_rtree_geometry *p,
  ಪೂರ್ಣಾಂಕ nCoord,
  sqlite3_rtree_dbl *aCoord,
  ಪೂರ್ಣಾಂಕ *piRes
){
  Cube *pCube = (Cube *)p->pUser;

  ಪ್ರತಿಜ್ಞಾ( p->pContext==(ಶೂನ್ಯ *)&g_ಇಲ್ಲಿ );

  ಆದರೆ( pCube==0 ){
    ಆದರೆ( p->nParam!=6 || nCoord!=6
     || p->aParam[3]<=0.0 || p->aParam[4]<=0.0 || p->aParam[5]<=0.0
    ){
      ಹಿಂತಿರುಗಿ SQLITE_ERROR;
    }
    pCube = (Cube *)sqlite3_malloc(ಗಾತ್ರ(Cube));
    ಆದರೆ( !pCube ){
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    }
    pCube->x = p->aParam[0];
    pCube->y = p->aParam[1];
    pCube->z = p->aParam[2];
    pCube->ಅಗಲ = p->aParam[3];
    pCube->ಎತ್ತರ = p->aParam[4];
    pCube->ಆಳ = p->aParam[5];

    p->pUser = (ಶೂನ್ಯ *)pCube;
    p->xDelUser = cube_context_free;
  }

  ಪ್ರತಿಜ್ಞಾ( nCoord==6 );
  *piRes = 0;
  ಆದರೆ( aCoord[0]<=(pCube->x+pCube->ಅಗಲ)
   && aCoord[1]>=pCube->x
   && aCoord[2]<=(pCube->y+pCube->ಎತ್ತರ)
   && aCoord[3]>=pCube->y
   && aCoord[4]<=(pCube->z+pCube->ಆಳ)
   && aCoord[5]>=pCube->z
  ){
    *piRes = 1;
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_RTREE */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI ರಿಜಿಸ್ಟರ್_cube_geom(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_ENABLE_RTREE
  ಬಳಸದ_PARAMETER(client_ದತ್ತಾಂಶ);
  ಬಳಸದ_PARAMETER(interp);
  ಬಳಸದ_PARAMETER(objc);
  ಬಳಸದ_PARAMETER(objv);
#ಇಲ್ಲದಿದ್ದರೆ
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, sqlite3**);
  ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);
  sqlite3 *db;
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "cube", cube_geom, (ಶೂನ್ಯ *)&g_ಇಲ್ಲಿ);
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_STATIC);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI ರಿಜಿಸ್ಟರ್_circle_geom(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_ENABLE_RTREE
  ಬಳಸದ_PARAMETER(client_ದತ್ತಾಂಶ);
  ಬಳಸದ_PARAMETER(interp);
  ಬಳಸದ_PARAMETER(objc);
  ಬಳಸದ_PARAMETER(objv);
#ಇಲ್ಲದಿದ್ದರೆ
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, sqlite3**);
  ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);
  sqlite3 *db;
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "circle", circle_geom, 0);
  ಆದರೆ( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "Qcircle",
                                      circle_query_func, 0, 0);
  }
  ಆದರೆ( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "breadthfirstsearch",
                                      bfs_query_func, 0, 0);
  }
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_STATIC);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಪೂರ್ಣಾಂಕ Sqlitetestrtree_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_cube_geom", ರಿಜಿಸ್ಟರ್_cube_geom, 0, 0);
  Tcl_CreateObjCommand(interp, "register_circle_geom",ರಿಜಿಸ್ಟರ್_circle_geom,0,0);
  ಹಿಂತಿರುಗಿ TCL_OK;
}
