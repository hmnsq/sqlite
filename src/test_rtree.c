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

#ifdef SQLITE_ENABLE_RTREE
/* 
** Type used to cache parameter information for the "circle" r-tree geometry
** callback.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ Circle Circle;
ಸಂರಚನಾ Circle {
  ಸಂರಚನಾ Box {
    ದ್ವಿಗುಣ xmin;
    ದ್ವಿಗುಣ xmax;
    ದ್ವಿಗುಣ ymin;
    ದ್ವಿಗುಣ ymax;
  } aBox[2];
  ದ್ವಿಗುಣ centerx;
  ದ್ವಿಗುಣ centery;
  ದ್ವಿಗುಣ radius;
  ದ್ವಿಗುಣ mxArea;
  ಪೂರ್ಣಾಙ್ಕ eScoreType;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ circle_geom(
  sqlite3_rtree_geometry *p,
  ಪೂರ್ಣಾಙ್ಕ nCoord, 
  sqlite3_rtree_dbl *aCoord,
  ಪೂರ್ಣಾಙ್ಕ *pRes
){
  ಪೂರ್ಣಾಙ್ಕ i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  ದ್ವಿಗುಣ xmin, xmax;              /* X dimensions of box being tested */
  ದ್ವಿಗುಣ ymin, ymax;              /* X dimensions of box being tested */

  xmin = aCoord[0];
  xmax = aCoord[1];
  ymin = aCoord[2];
  ymax = aCoord[3];
  pCircle = (Circle *)p->pUser;
  ಯದಿ( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    ಯದಿ( nCoord!=4 ) ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;

    /* Test that the correct number of parameters (3) have been supplied,
    ** and that the parameters are in range (that the radius of the circle 
    ** radius is greater than zero). */
    ಯದಿ( p->nParam!=3 || p->aParam[2]<0.0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(ಪರಿಮಾಣ(Circle)));
    ಯದಿ( !pCircle ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
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
  ಪ್ರತಿ(i=0; i<4; i++){
    ದ್ವಿಗುಣ x = (i&0x01) ? xmax : xmin;
    ದ್ವಿಗುಣ y = (i&0x02) ? ymax : ymin;
    ದ್ವಿಗುಣ d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    ಯದಿ( d2<(pCircle->radius*pCircle->radius) ){
      *pRes = 1;
      ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
    }
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  ಪ್ರತಿ(i=0; i<2; i++){
    ಯದಿ( xmin<=pCircle->aBox[i].xmin 
     && xmax>=pCircle->aBox[i].xmax 
     && ymin<=pCircle->aBox[i].ymin 
     && ymax>=pCircle->aBox[i].ymax 
    ){
      *pRes = 1;
      ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
    }
  }

  /* The specified bounding box does not intersect the circular region. Set
  ** the output variable to zero and return SQLITE_OK. */
  *pRes = 0;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ circle_query_func(sqlite3_rtree_query_info *p){
  ಪೂರ್ಣಾಙ್ಕ i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  ದ್ವಿಗುಣ xmin, xmax;              /* X dimensions of box being tested */
  ದ್ವಿಗುಣ ymin, ymax;              /* X dimensions of box being tested */
  ಪೂರ್ಣಾಙ್ಕ nWithin = 0;                /* Number of corners inside the circle */

  xmin = p->aCoord[0];
  xmax = p->aCoord[1];
  ymin = p->aCoord[2];
  ymax = p->aCoord[3];
  pCircle = (Circle *)p->pUser;
  ಯದಿ( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    ಯದಿ( p->nCoord!=4 ) ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;

    /* Test that the correct number of parameters (1 or 4) have been supplied.
    */
    ಯದಿ( p->nParam!=4 && p->nParam!=1 ) ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(ಪರಿಮಾಣ(Circle)));
    ಯದಿ( !pCircle ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    p->xDelUser = circle_del;

    /* Record the center and radius of the circular region. One way that
    ** tested bounding boxes that intersect the circular region are detected
    ** is by testing if each corner of the bounding box lies within radius
    ** units of the center of the circle. */
    ಯದಿ( p->nParam==4 ){
      pCircle->centerx = p->aParam[0];
      pCircle->centery = p->aParam[1];
      pCircle->radius = p->aParam[2];
      pCircle->eScoreType = (ಪೂರ್ಣಾಙ್ಕ)p->aParam[3];
    }ಅನ್ಯಥಾ{
      ಧ್ರುವ ವರ್ಣ *z = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(p->apSqlParam[0]);
      pCircle->centerx = 0.0;
      pCircle->centery = 0.0;
      pCircle->radius = 0.0;
      pCircle->eScoreType = 0;
      ಯಾವತ್( z && z[0] ){
        ಯದಿ( z[0]=='r' && z[1]==':' ){
          pCircle->radius = atof(&z[2]);
        }ಅನ್ಯಥಾ ಯದಿ( z[0]=='x' && z[1]==':' ){
          pCircle->centerx = atof(&z[2]);
        }ಅನ್ಯಥಾ ಯದಿ( z[0]=='y' && z[1]==':' ){
          pCircle->centery = atof(&z[2]);
        }ಅನ್ಯಥಾ ಯದಿ( z[0]=='e' && z[1]==':' ){
          pCircle->eScoreType = (ಪೂರ್ಣಾಙ್ಕ)atof(&z[2]);
        }ಅನ್ಯಥಾ ಯದಿ( z[0]==' ' ){
          z++;
          ಅನುವರ್ತ;
        }
        ಯಾವತ್( z[0]!=0 && z[0]!=' ' ) z++;
        ಯಾವತ್( z[0]==' ' ) z++;
      }
    }
    ಯದಿ( pCircle->radius<0.0 ){
      sqlite3_free(pCircle);
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
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
  ಪ್ರತಿ(i=0; i<4; i++){
    ದ್ವಿಗುಣ x = (i&0x01) ? xmax : xmin;
    ದ್ವಿಗುಣ y = (i&0x02) ? ymax : ymin;
    ದ್ವಿಗುಣ d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    ಯದಿ( d2<(pCircle->radius*pCircle->radius) ) nWithin++;
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  ಯದಿ( nWithin==0 ){
    ಪ್ರತಿ(i=0; i<2; i++){
      ಯದಿ( xmin<=pCircle->aBox[i].xmin 
       && xmax>=pCircle->aBox[i].xmax 
       && ymin<=pCircle->aBox[i].ymin 
       && ymax>=pCircle->aBox[i].ymax 
      ){
        nWithin = 1;
        ಭಙ್ಗ;
      }
    }
  }

  ಯದಿ( pCircle->eScoreType==1 ){
    /* Depth first search */
    p->rScore = p->iLevel;
  }ಅನ್ಯಥಾ ಯದಿ( pCircle->eScoreType==2 ){
    /* Breadth first search */
    p->rScore = 100 - p->iLevel;
  }ಅನ್ಯಥಾ ಯದಿ( pCircle->eScoreType==3 ){
    /* Depth-first search, except sort the leaf nodes by area with
    ** the largest area first */
    ಯದಿ( p->iLevel==1 ){
      p->rScore = 1.0 - (xmax-xmin)*(ymax-ymin)/pCircle->mxArea;
      ಯದಿ( p->rScore<0.01 ) p->rScore = 0.01;
    }ಅನ್ಯಥಾ{
      p->rScore = 0.0;
    }
  }ಅನ್ಯಥಾ ಯದಿ( pCircle->eScoreType==4 ){
    /* Depth-first search, except exclude odd rowids */
    p->rScore = p->iLevel;
    ಯದಿ( p->iRowid&1 ) nWithin = 0;
  }ಅನ್ಯಥಾ{
    /* Breadth-first search, except exclude odd rowids */
    p->rScore = 100 - p->iLevel;
    ಯದಿ( p->iRowid&1 ) nWithin = 0;
  }
  ಯದಿ( nWithin==0 ){
    p->eWithin = NOT_WITHIN;
  }ಅನ್ಯಥಾ ಯದಿ( nWithin>=4 ){
    p->eWithin = FULLY_WITHIN;
  }ಅನ್ಯಥಾ{
    p->eWithin = PARTLY_WITHIN;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
/*
** Implementation of "breadthfirstsearch" r-tree geometry callback using the 
** 2nd-generation interface that allows scoring.
**
**     ... WHERE id MATCH breadthfirstsearch($x0,$x1,$y0,$y1) ...
**
** It returns all entries whose bounding boxes overlap with $x0,$x1,$y0,$y1.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bfs_query_func(sqlite3_rtree_query_info *p){
  ದ್ವಿಗುಣ x0,x1,y0,y1;        /* Dimensions of box being tested */
  ದ್ವಿಗುಣ bx0,bx1,by0,by1;    /* Boundary of the query function */

  ಯದಿ( p->nParam!=4 ) ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  x0 = p->aCoord[0];
  x1 = p->aCoord[1];
  y0 = p->aCoord[2];
  y1 = p->aCoord[3];
  bx0 = p->aParam[0];
  bx1 = p->aParam[1];
  by0 = p->aParam[2];
  by1 = p->aParam[3];
  p->rScore = 100 - p->iLevel;
  ಯದಿ( p->eParentWithin==FULLY_WITHIN ){
    p->eWithin = FULLY_WITHIN;
  }ಅನ್ಯಥಾ ಯದಿ( x0>=bx0 && x1<=bx1 && y0>=by0 && y1<=by1 ){
    p->eWithin = FULLY_WITHIN;
  }ಅನ್ಯಥಾ ಯದಿ( x1>=bx0 && x0<=bx1 && y1>=by0 && y0<=by1 ){
    p->eWithin = PARTLY_WITHIN;
  }ಅನ್ಯಥಾ{
    p->eWithin = NOT_WITHIN;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* END of implementation of "circle" geometry callback.
**************************************************************************
*************************************************************************/

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ Cube Cube;
ಸಂರಚನಾ Cube {
  ದ್ವಿಗುಣ x;
  ದ್ವಿಗುಣ y;
  ದ್ವಿಗುಣ z;
  ದ್ವಿಗುಣ width;
  ದ್ವಿಗುಣ height;
  ದ್ವಿಗುಣ depth;
};

ಸ್ಥಿರ ಶೂನ್ಯ cube_context_free(ಶೂನ್ಯ *p){
  sqlite3_free(p);
}

/*
** The context pointer registered along with the 'cube' callback is
** always ((void *)&gHere). This is just to facilitate testing, it is not
** actually used for anything.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ gHere = 42;

/*
** Implementation of a simple r-tree geom callback to test for intersection
** of r-tree rows with a "cube" shape. Cubes are defined by six scalar
** coordinates as follows:
**
**   cube(x, y, z, width, height, depth)
**
** The width, height and depth parameters must all be greater than zero.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cube_geom(
  sqlite3_rtree_geometry *p,
  ಪೂರ್ಣಾಙ್ಕ nCoord,
  sqlite3_rtree_dbl *aCoord,
  ಪೂರ್ಣಾಙ್ಕ *piRes
){
  Cube *pCube = (Cube *)p->pUser;

  ಪ್ರತಿಜ್ಞಾ( p->pContext==(ಶೂನ್ಯ *)&gHere );

  ಯದಿ( pCube==0 ){
    ಯದಿ( p->nParam!=6 || nCoord!=6
     || p->aParam[3]<=0.0 || p->aParam[4]<=0.0 || p->aParam[5]<=0.0
    ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
    }
    pCube = (Cube *)sqlite3_malloc(ಪರಿಮಾಣ(Cube));
    ಯದಿ( !pCube ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    }
    pCube->x = p->aParam[0];
    pCube->y = p->aParam[1];
    pCube->z = p->aParam[2];
    pCube->width = p->aParam[3];
    pCube->height = p->aParam[4];
    pCube->depth = p->aParam[5];

    p->pUser = (ಶೂನ್ಯ *)pCube;
    p->xDelUser = cube_context_free;
  }

  ಪ್ರತಿಜ್ಞಾ( nCoord==6 );
  *piRes = 0;
  ಯದಿ( aCoord[0]<=(pCube->x+pCube->width)
   && aCoord[1]>=pCube->x
   && aCoord[2]<=(pCube->y+pCube->height)
   && aCoord[3]>=pCube->y
   && aCoord[4]<=(pCube->z+pCube->depth)
   && aCoord[5]>=pCube->z
  ){
    *piRes = 1;
  }

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
#endif /* SQLITE_ENABLE_RTREE */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI register_cube_geom(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_ENABLE_RTREE
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(interp);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
#ಅನ್ಯಥಾ
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp*, ಧ್ರುವ ವರ್ಣ*, sqlite3**);
  ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3ErrName(ಪೂರ್ಣಾಙ್ಕ);
  sqlite3 *db;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "cube", cube_geom, (ಶೂನ್ಯ *)&gHere);
  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_STATIC);
#endif
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI register_circle_geom(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_ENABLE_RTREE
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(interp);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
#ಅನ್ಯಥಾ
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp*, ಧ್ರುವ ವರ್ಣ*, sqlite3**);
  ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3ErrName(ಪೂರ್ಣಾಙ್ಕ);
  sqlite3 *db;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "circle", circle_geom, 0);
  ಯದಿ( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "Qcircle",
                                      circle_query_func, 0, 0);
  }
  ಯದಿ( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "breadthfirstsearch",
                                      bfs_query_func, 0, 0);
  }
  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_STATIC);
#endif
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಪೂರ್ಣಾಙ್ಕ Sqlitetestrtree_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_cube_geom", register_cube_geom, 0, 0);
  Tcl_CreateObjCommand(interp, "register_circle_geom",register_circle_geom,0,0);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
