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

#അന്തര്ഭാവ "sqlite3.h"
#അന്തര്ഭാവ "tclsqlite.h"

/* Solely for the UNUSED_PARAMETER() macro. */
#അന്തര്ഭാവ "sqliteInt.h"

#ifdef SQLITE_ENABLE_RTREE
/* 
** Type used to cache parameter information for the "circle" r-tree geometry
** callback.
*/
തരനിർവചനം ഘടന Circle Circle;
ഘടന Circle {
  ഘടന Box {
    ഇരട്ട xmin;
    ഇരട്ട xmax;
    ഇരട്ട ymin;
    ഇരട്ട ymax;
  } aBox[2];
  ഇരട്ട centerx;
  ഇരട്ട centery;
  ഇരട്ട radius;
  ഇരട്ട mxArea;
  പൂർണ്ണസംഖ്യ eScoreType;
};

/*
** Destructor function for Circle objects allocated by circle_geom().
*/
സ്ഥിതം ശൂന്യം circle_del(ശൂന്യം *p){
  sqlite3_free(p);
}

/*
** Implementation of "circle" r-tree geometry callback.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ circle_geom(
  sqlite3_rtree_geometry *p,
  പൂർണ്ണസംഖ്യ nCoord, 
  sqlite3_rtree_dbl *aCoord,
  പൂർണ്ണസംഖ്യ *pRes
){
  പൂർണ്ണസംഖ്യ i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  ഇരട്ട xmin, xmax;              /* X dimensions of box being tested */
  ഇരട്ട ymin, ymax;              /* X dimensions of box being tested */

  xmin = aCoord[0];
  xmax = aCoord[1];
  ymin = aCoord[2];
  ymax = aCoord[3];
  pCircle = (Circle *)p->pUser;
  എങ്കിൽ( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    എങ്കിൽ( nCoord!=4 ) തിരികെ SQLITE_ERROR;

    /* Test that the correct number of parameters (3) have been supplied,
    ** and that the parameters are in range (that the radius of the circle 
    ** radius is greater than zero). */
    എങ്കിൽ( p->nParam!=3 || p->aParam[2]<0.0 ) തിരികെ SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(വലുപ്പം(Circle)));
    എങ്കിൽ( !pCircle ) തിരികെ SQLITE_NOMEM;
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
  വേണ്ടി(i=0; i<4; i++){
    ഇരട്ട x = (i&0x01) ? xmax : xmin;
    ഇരട്ട y = (i&0x02) ? ymax : ymin;
    ഇരട്ട d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    എങ്കിൽ( d2<(pCircle->radius*pCircle->radius) ){
      *pRes = 1;
      തിരികെ SQLITE_OK;
    }
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  വേണ്ടി(i=0; i<2; i++){
    എങ്കിൽ( xmin<=pCircle->aBox[i].xmin 
     && xmax>=pCircle->aBox[i].xmax 
     && ymin<=pCircle->aBox[i].ymin 
     && ymax>=pCircle->aBox[i].ymax 
    ){
      *pRes = 1;
      തിരികെ SQLITE_OK;
    }
  }

  /* The specified bounding box does not intersect the circular region. Set
  ** the output variable to zero and return SQLITE_OK. */
  *pRes = 0;
  തിരികെ SQLITE_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ circle_query_func(sqlite3_rtree_query_info *p){
  പൂർണ്ണസംഖ്യ i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  ഇരട്ട xmin, xmax;              /* X dimensions of box being tested */
  ഇരട്ട ymin, ymax;              /* X dimensions of box being tested */
  പൂർണ്ണസംഖ്യ nWithin = 0;                /* Number of corners inside the circle */

  xmin = p->aCoord[0];
  xmax = p->aCoord[1];
  ymin = p->aCoord[2];
  ymax = p->aCoord[3];
  pCircle = (Circle *)p->pUser;
  എങ്കിൽ( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    എങ്കിൽ( p->nCoord!=4 ) തിരികെ SQLITE_ERROR;

    /* Test that the correct number of parameters (1 or 4) have been supplied.
    */
    എങ്കിൽ( p->nParam!=4 && p->nParam!=1 ) തിരികെ SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(വലുപ്പം(Circle)));
    എങ്കിൽ( !pCircle ) തിരികെ SQLITE_NOMEM;
    p->xDelUser = circle_del;

    /* Record the center and radius of the circular region. One way that
    ** tested bounding boxes that intersect the circular region are detected
    ** is by testing if each corner of the bounding box lies within radius
    ** units of the center of the circle. */
    എങ്കിൽ( p->nParam==4 ){
      pCircle->centerx = p->aParam[0];
      pCircle->centery = p->aParam[1];
      pCircle->radius = p->aParam[2];
      pCircle->eScoreType = (പൂർണ്ണസംഖ്യ)p->aParam[3];
    }അല്ലെങ്കിൽ{
      സ്ഥിരം അക്ഷരം *z = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(p->apSqlParam[0]);
      pCircle->centerx = 0.0;
      pCircle->centery = 0.0;
      pCircle->radius = 0.0;
      pCircle->eScoreType = 0;
      ആകുമ്പോൾ( z && z[0] ){
        എങ്കിൽ( z[0]=='r' && z[1]==':' ){
          pCircle->radius = atof(&z[2]);
        }അല്ലെങ്കിൽ എങ്കിൽ( z[0]=='x' && z[1]==':' ){
          pCircle->centerx = atof(&z[2]);
        }അല്ലെങ്കിൽ എങ്കിൽ( z[0]=='y' && z[1]==':' ){
          pCircle->centery = atof(&z[2]);
        }അല്ലെങ്കിൽ എങ്കിൽ( z[0]=='e' && z[1]==':' ){
          pCircle->eScoreType = (പൂർണ്ണസംഖ്യ)atof(&z[2]);
        }അല്ലെങ്കിൽ എങ്കിൽ( z[0]==' ' ){
          z++;
          തുടരുക;
        }
        ആകുമ്പോൾ( z[0]!=0 && z[0]!=' ' ) z++;
        ആകുമ്പോൾ( z[0]==' ' ) z++;
      }
    }
    എങ്കിൽ( pCircle->radius<0.0 ){
      sqlite3_free(pCircle);
      തിരികെ SQLITE_NOMEM;
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
  വേണ്ടി(i=0; i<4; i++){
    ഇരട്ട x = (i&0x01) ? xmax : xmin;
    ഇരട്ട y = (i&0x02) ? ymax : ymin;
    ഇരട്ട d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    എങ്കിൽ( d2<(pCircle->radius*pCircle->radius) ) nWithin++;
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  എങ്കിൽ( nWithin==0 ){
    വേണ്ടി(i=0; i<2; i++){
      എങ്കിൽ( xmin<=pCircle->aBox[i].xmin 
       && xmax>=pCircle->aBox[i].xmax 
       && ymin<=pCircle->aBox[i].ymin 
       && ymax>=pCircle->aBox[i].ymax 
      ){
        nWithin = 1;
        നിർത്തുക;
      }
    }
  }

  എങ്കിൽ( pCircle->eScoreType==1 ){
    /* Depth first search */
    p->rScore = p->iLevel;
  }അല്ലെങ്കിൽ എങ്കിൽ( pCircle->eScoreType==2 ){
    /* Breadth first search */
    p->rScore = 100 - p->iLevel;
  }അല്ലെങ്കിൽ എങ്കിൽ( pCircle->eScoreType==3 ){
    /* Depth-first search, except sort the leaf nodes by area with
    ** the largest area first */
    എങ്കിൽ( p->iLevel==1 ){
      p->rScore = 1.0 - (xmax-xmin)*(ymax-ymin)/pCircle->mxArea;
      എങ്കിൽ( p->rScore<0.01 ) p->rScore = 0.01;
    }അല്ലെങ്കിൽ{
      p->rScore = 0.0;
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( pCircle->eScoreType==4 ){
    /* Depth-first search, except exclude odd rowids */
    p->rScore = p->iLevel;
    എങ്കിൽ( p->iRowid&1 ) nWithin = 0;
  }അല്ലെങ്കിൽ{
    /* Breadth-first search, except exclude odd rowids */
    p->rScore = 100 - p->iLevel;
    എങ്കിൽ( p->iRowid&1 ) nWithin = 0;
  }
  എങ്കിൽ( nWithin==0 ){
    p->eWithin = NOT_WITHIN;
  }അല്ലെങ്കിൽ എങ്കിൽ( nWithin>=4 ){
    p->eWithin = FULLY_WITHIN;
  }അല്ലെങ്കിൽ{
    p->eWithin = PARTLY_WITHIN;
  }
  തിരികെ SQLITE_OK;
}
/*
** Implementation of "breadthfirstsearch" r-tree geometry callback using the 
** 2nd-generation interface that allows scoring.
**
**     ... WHERE id MATCH breadthfirstsearch($x0,$x1,$y0,$y1) ...
**
** It returns all entries whose bounding boxes overlap with $x0,$x1,$y0,$y1.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ bfs_query_func(sqlite3_rtree_query_info *p){
  ഇരട്ട x0,x1,y0,y1;        /* Dimensions of box being tested */
  ഇരട്ട bx0,bx1,by0,by1;    /* Boundary of the query function */

  എങ്കിൽ( p->nParam!=4 ) തിരികെ SQLITE_ERROR;
  x0 = p->aCoord[0];
  x1 = p->aCoord[1];
  y0 = p->aCoord[2];
  y1 = p->aCoord[3];
  bx0 = p->aParam[0];
  bx1 = p->aParam[1];
  by0 = p->aParam[2];
  by1 = p->aParam[3];
  p->rScore = 100 - p->iLevel;
  എങ്കിൽ( p->eParentWithin==FULLY_WITHIN ){
    p->eWithin = FULLY_WITHIN;
  }അല്ലെങ്കിൽ എങ്കിൽ( x0>=bx0 && x1<=bx1 && y0>=by0 && y1<=by1 ){
    p->eWithin = FULLY_WITHIN;
  }അല്ലെങ്കിൽ എങ്കിൽ( x1>=bx0 && x0<=bx1 && y1>=by0 && y0<=by1 ){
    p->eWithin = PARTLY_WITHIN;
  }അല്ലെങ്കിൽ{
    p->eWithin = NOT_WITHIN;
  }
  തിരികെ SQLITE_OK;
}

/* END of implementation of "circle" geometry callback.
**************************************************************************
*************************************************************************/

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ "tclsqlite.h"

തരനിർവചനം ഘടന Cube Cube;
ഘടന Cube {
  ഇരട്ട x;
  ഇരട്ട y;
  ഇരട്ട z;
  ഇരട്ട width;
  ഇരട്ട height;
  ഇരട്ട depth;
};

സ്ഥിതം ശൂന്യം cube_context_free(ശൂന്യം *p){
  sqlite3_free(p);
}

/*
** The context pointer registered along with the 'cube' callback is
** always ((void *)&gHere). This is just to facilitate testing, it is not
** actually used for anything.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ gHere = 42;

/*
** Implementation of a simple r-tree geom callback to test for intersection
** of r-tree rows with a "cube" shape. Cubes are defined by six scalar
** coordinates as follows:
**
**   cube(x, y, z, width, height, depth)
**
** The width, height and depth parameters must all be greater than zero.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ cube_geom(
  sqlite3_rtree_geometry *p,
  പൂർണ്ണസംഖ്യ nCoord,
  sqlite3_rtree_dbl *aCoord,
  പൂർണ്ണസംഖ്യ *piRes
){
  Cube *pCube = (Cube *)p->pUser;

  പ്രതിജ്ഞാ( p->pContext==(ശൂന്യം *)&gHere );

  എങ്കിൽ( pCube==0 ){
    എങ്കിൽ( p->nParam!=6 || nCoord!=6
     || p->aParam[3]<=0.0 || p->aParam[4]<=0.0 || p->aParam[5]<=0.0
    ){
      തിരികെ SQLITE_ERROR;
    }
    pCube = (Cube *)sqlite3_malloc(വലുപ്പം(Cube));
    എങ്കിൽ( !pCube ){
      തിരികെ SQLITE_NOMEM;
    }
    pCube->x = p->aParam[0];
    pCube->y = p->aParam[1];
    pCube->z = p->aParam[2];
    pCube->width = p->aParam[3];
    pCube->height = p->aParam[4];
    pCube->depth = p->aParam[5];

    p->pUser = (ശൂന്യം *)pCube;
    p->xDelUser = cube_context_free;
  }

  പ്രതിജ്ഞാ( nCoord==6 );
  *piRes = 0;
  എങ്കിൽ( aCoord[0]<=(pCube->x+pCube->width)
   && aCoord[1]>=pCube->x
   && aCoord[2]<=(pCube->y+pCube->height)
   && aCoord[3]>=pCube->y
   && aCoord[4]<=(pCube->z+pCube->depth)
   && aCoord[5]>=pCube->z
  ){
    *piRes = 1;
  }

  തിരികെ SQLITE_OK;
}
#endif /* SQLITE_ENABLE_RTREE */

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI register_cube_geom(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_ENABLE_RTREE
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(interp);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
#അല്ലെങ്കിൽ
  ബാഹ്യ പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp*, സ്ഥിരം അക്ഷരം*, sqlite3**);
  ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);
  sqlite3 *db;
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "cube", cube_geom, (ശൂന്യം *)&gHere);
  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_STATIC);
#endif
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI register_circle_geom(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_ENABLE_RTREE
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(interp);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
#അല്ലെങ്കിൽ
  ബാഹ്യ പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp*, സ്ഥിരം അക്ഷരം*, sqlite3**);
  ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);
  sqlite3 *db;
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "circle", circle_geom, 0);
  എങ്കിൽ( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "Qcircle",
                                      circle_query_func, 0, 0);
  }
  എങ്കിൽ( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "breadthfirstsearch",
                                      bfs_query_func, 0, 0);
  }
  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_STATIC);
#endif
  തിരികെ TCL_OK;
}

പൂർണ്ണസംഖ്യ Sqlitetestrtree_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_cube_geom", register_cube_geom, 0, 0);
  Tcl_CreateObjCommand(interp, "register_circle_geom",register_circle_geom,0,0);
  തിരികെ TCL_OK;
}
