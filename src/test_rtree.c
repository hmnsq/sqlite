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

#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

/* Solely for the UNUSED_PARAMETER() macro. */
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_RTREE
/* 
** Type used to cache parameter information for the "circle" r-tree geometry
** callback.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Circle Circle;
ਬਣਤਰ Circle {
  ਬਣਤਰ Box {
    ਦੁੱਗਣਾ xmin;
    ਦੁੱਗਣਾ xmax;
    ਦੁੱਗਣਾ ymin;
    ਦੁੱਗਣਾ ymax;
  } aBox[2];
  ਦੁੱਗਣਾ centerx;
  ਦੁੱਗਣਾ centery;
  ਦੁੱਗਣਾ radius;
  ਦੁੱਗਣਾ mxArea;
  ਪੂਰਨ eScoreType;
};

/*
** Destructor function for Circle objects allocated by circle_geom().
*/
ਸਥਿਰ ਖਾਲੀ circle_del(ਖਾਲੀ *p){
  sqlite3_free(p);
}

/*
** Implementation of "circle" r-tree geometry callback.
*/
ਸਥਿਰ ਪੂਰਨ circle_geom(
  sqlite3_rtree_geometry *p,
  ਪੂਰਨ nCoord, 
  sqlite3_rtree_dbl *aCoord,
  ਪੂਰਨ *pRes
){
  ਪੂਰਨ i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  ਦੁੱਗਣਾ xmin, xmax;              /* X dimensions of box being tested */
  ਦੁੱਗਣਾ ymin, ymax;              /* X dimensions of box being tested */

  xmin = aCoord[0];
  xmax = aCoord[1];
  ymin = aCoord[2];
  ymax = aCoord[3];
  pCircle = (Circle *)p->pUser;
  ਜੇ( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    ਜੇ( nCoord!=4 ) ਵਾਪਸ SQLITE_ERROR;

    /* Test that the correct number of parameters (3) have been supplied,
    ** and that the parameters are in range (that the radius of the circle 
    ** radius is greater than zero). */
    ਜੇ( p->nParam!=3 || p->aParam[2]<0.0 ) ਵਾਪਸ SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(ਆਕਾਰ(Circle)));
    ਜੇ( !pCircle ) ਵਾਪਸ SQLITE_NOMEM;
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
  ਲਈ(i=0; i<4; i++){
    ਦੁੱਗਣਾ x = (i&0x01) ? xmax : xmin;
    ਦੁੱਗਣਾ y = (i&0x02) ? ymax : ymin;
    ਦੁੱਗਣਾ d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    ਜੇ( d2<(pCircle->radius*pCircle->radius) ){
      *pRes = 1;
      ਵਾਪਸ SQLITE_OK;
    }
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  ਲਈ(i=0; i<2; i++){
    ਜੇ( xmin<=pCircle->aBox[i].xmin 
     && xmax>=pCircle->aBox[i].xmax 
     && ymin<=pCircle->aBox[i].ymin 
     && ymax>=pCircle->aBox[i].ymax 
    ){
      *pRes = 1;
      ਵਾਪਸ SQLITE_OK;
    }
  }

  /* The specified bounding box does not intersect the circular region. Set
  ** the output variable to zero and return SQLITE_OK. */
  *pRes = 0;
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ circle_query_func(sqlite3_rtree_query_info *p){
  ਪੂਰਨ i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  ਦੁੱਗਣਾ xmin, xmax;              /* X dimensions of box being tested */
  ਦੁੱਗਣਾ ymin, ymax;              /* X dimensions of box being tested */
  ਪੂਰਨ nWithin = 0;                /* Number of corners inside the circle */

  xmin = p->aCoord[0];
  xmax = p->aCoord[1];
  ymin = p->aCoord[2];
  ymax = p->aCoord[3];
  pCircle = (Circle *)p->pUser;
  ਜੇ( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    ਜੇ( p->nCoord!=4 ) ਵਾਪਸ SQLITE_ERROR;

    /* Test that the correct number of parameters (1 or 4) have been supplied.
    */
    ਜੇ( p->nParam!=4 && p->nParam!=1 ) ਵਾਪਸ SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(ਆਕਾਰ(Circle)));
    ਜੇ( !pCircle ) ਵਾਪਸ SQLITE_NOMEM;
    p->xDelUser = circle_del;

    /* Record the center and radius of the circular region. One way that
    ** tested bounding boxes that intersect the circular region are detected
    ** is by testing if each corner of the bounding box lies within radius
    ** units of the center of the circle. */
    ਜੇ( p->nParam==4 ){
      pCircle->centerx = p->aParam[0];
      pCircle->centery = p->aParam[1];
      pCircle->radius = p->aParam[2];
      pCircle->eScoreType = (ਪੂਰਨ)p->aParam[3];
    }ਨਹੀਂਤਾਂ{
      ਅਚਲ ਅੱਖਰ *z = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(p->apSqlParam[0]);
      pCircle->centerx = 0.0;
      pCircle->centery = 0.0;
      pCircle->radius = 0.0;
      pCircle->eScoreType = 0;
      ਜਦੋਂ( z && z[0] ){
        ਜੇ( z[0]=='r' && z[1]==':' ){
          pCircle->radius = atof(&z[2]);
        }ਨਹੀਂਤਾਂ ਜੇ( z[0]=='x' && z[1]==':' ){
          pCircle->centerx = atof(&z[2]);
        }ਨਹੀਂਤਾਂ ਜੇ( z[0]=='y' && z[1]==':' ){
          pCircle->centery = atof(&z[2]);
        }ਨਹੀਂਤਾਂ ਜੇ( z[0]=='e' && z[1]==':' ){
          pCircle->eScoreType = (ਪੂਰਨ)atof(&z[2]);
        }ਨਹੀਂਤਾਂ ਜੇ( z[0]==' ' ){
          z++;
          ਜਾਰੀ;
        }
        ਜਦੋਂ( z[0]!=0 && z[0]!=' ' ) z++;
        ਜਦੋਂ( z[0]==' ' ) z++;
      }
    }
    ਜੇ( pCircle->radius<0.0 ){
      sqlite3_free(pCircle);
      ਵਾਪਸ SQLITE_NOMEM;
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
  ਲਈ(i=0; i<4; i++){
    ਦੁੱਗਣਾ x = (i&0x01) ? xmax : xmin;
    ਦੁੱਗਣਾ y = (i&0x02) ? ymax : ymin;
    ਦੁੱਗਣਾ d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    ਜੇ( d2<(pCircle->radius*pCircle->radius) ) nWithin++;
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  ਜੇ( nWithin==0 ){
    ਲਈ(i=0; i<2; i++){
      ਜੇ( xmin<=pCircle->aBox[i].xmin 
       && xmax>=pCircle->aBox[i].xmax 
       && ymin<=pCircle->aBox[i].ymin 
       && ymax>=pCircle->aBox[i].ymax 
      ){
        nWithin = 1;
        ਰੋਕੋ;
      }
    }
  }

  ਜੇ( pCircle->eScoreType==1 ){
    /* Depth first search */
    p->rScore = p->iLevel;
  }ਨਹੀਂਤਾਂ ਜੇ( pCircle->eScoreType==2 ){
    /* Breadth first search */
    p->rScore = 100 - p->iLevel;
  }ਨਹੀਂਤਾਂ ਜੇ( pCircle->eScoreType==3 ){
    /* Depth-first search, except sort the leaf nodes by area with
    ** the largest area first */
    ਜੇ( p->iLevel==1 ){
      p->rScore = 1.0 - (xmax-xmin)*(ymax-ymin)/pCircle->mxArea;
      ਜੇ( p->rScore<0.01 ) p->rScore = 0.01;
    }ਨਹੀਂਤਾਂ{
      p->rScore = 0.0;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( pCircle->eScoreType==4 ){
    /* Depth-first search, except exclude odd rowids */
    p->rScore = p->iLevel;
    ਜੇ( p->iRowid&1 ) nWithin = 0;
  }ਨਹੀਂਤਾਂ{
    /* Breadth-first search, except exclude odd rowids */
    p->rScore = 100 - p->iLevel;
    ਜੇ( p->iRowid&1 ) nWithin = 0;
  }
  ਜੇ( nWithin==0 ){
    p->eWithin = NOT_WITHIN;
  }ਨਹੀਂਤਾਂ ਜੇ( nWithin>=4 ){
    p->eWithin = FULLY_WITHIN;
  }ਨਹੀਂਤਾਂ{
    p->eWithin = PARTLY_WITHIN;
  }
  ਵਾਪਸ SQLITE_OK;
}
/*
** Implementation of "breadthfirstsearch" r-tree geometry callback using the 
** 2nd-generation interface that allows scoring.
**
**     ... WHERE id MATCH breadthfirstsearch($x0,$x1,$y0,$y1) ...
**
** It returns all entries whose bounding boxes overlap with $x0,$x1,$y0,$y1.
*/
ਸਥਿਰ ਪੂਰਨ bfs_query_func(sqlite3_rtree_query_info *p){
  ਦੁੱਗਣਾ x0,x1,y0,y1;        /* Dimensions of box being tested */
  ਦੁੱਗਣਾ bx0,bx1,by0,by1;    /* Boundary of the query function */

  ਜੇ( p->nParam!=4 ) ਵਾਪਸ SQLITE_ERROR;
  x0 = p->aCoord[0];
  x1 = p->aCoord[1];
  y0 = p->aCoord[2];
  y1 = p->aCoord[3];
  bx0 = p->aParam[0];
  bx1 = p->aParam[1];
  by0 = p->aParam[2];
  by1 = p->aParam[3];
  p->rScore = 100 - p->iLevel;
  ਜੇ( p->e_ਮਾਪਾ_Within==FULLY_WITHIN ){
    p->eWithin = FULLY_WITHIN;
  }ਨਹੀਂਤਾਂ ਜੇ( x0>=bx0 && x1<=bx1 && y0>=by0 && y1<=by1 ){
    p->eWithin = FULLY_WITHIN;
  }ਨਹੀਂਤਾਂ ਜੇ( x1>=bx0 && x0<=bx1 && y1>=by0 && y0<=by1 ){
    p->eWithin = PARTLY_WITHIN;
  }ਨਹੀਂਤਾਂ{
    p->eWithin = NOT_WITHIN;
  }
  ਵਾਪਸ SQLITE_OK;
}

/* END of implementation of "circle" geometry callback.
**************************************************************************
*************************************************************************/

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Cube Cube;
ਬਣਤਰ Cube {
  ਦੁੱਗਣਾ x;
  ਦੁੱਗਣਾ y;
  ਦੁੱਗਣਾ z;
  ਦੁੱਗਣਾ ਚੌੜਾਈ;
  ਦੁੱਗਣਾ ਉਚਾਈ;
  ਦੁੱਗਣਾ ਡੂੰਘਾਈ;
};

ਸਥਿਰ ਖਾਲੀ cube_context_free(ਖਾਲੀ *p){
  sqlite3_free(p);
}

/*
** The context pointer registered along with the 'cube' callback is
** always ((void *)&gHere). This is just to facilitate testing, it is not
** actually used for anything.
*/
ਸਥਿਰ ਪੂਰਨ g_ਇੱਥੇ = 42;

/*
** Implementation of a simple r-tree geom callback to test for intersection
** of r-tree rows with a "cube" shape. Cubes are defined by six scalar
** coordinates as follows:
**
**   cube(x, y, z, width, height, depth)
**
** The width, height and depth parameters must all be greater than zero.
*/
ਸਥਿਰ ਪੂਰਨ cube_geom(
  sqlite3_rtree_geometry *p,
  ਪੂਰਨ nCoord,
  sqlite3_rtree_dbl *aCoord,
  ਪੂਰਨ *piRes
){
  Cube *pCube = (Cube *)p->pUser;

  ਪ੍ਰਤਿਜ੍ਞਾ( p->pContext==(ਖਾਲੀ *)&g_ਇੱਥੇ );

  ਜੇ( pCube==0 ){
    ਜੇ( p->nParam!=6 || nCoord!=6
     || p->aParam[3]<=0.0 || p->aParam[4]<=0.0 || p->aParam[5]<=0.0
    ){
      ਵਾਪਸ SQLITE_ERROR;
    }
    pCube = (Cube *)sqlite3_malloc(ਆਕਾਰ(Cube));
    ਜੇ( !pCube ){
      ਵਾਪਸ SQLITE_NOMEM;
    }
    pCube->x = p->aParam[0];
    pCube->y = p->aParam[1];
    pCube->z = p->aParam[2];
    pCube->ਚੌੜਾਈ = p->aParam[3];
    pCube->ਉਚਾਈ = p->aParam[4];
    pCube->ਡੂੰਘਾਈ = p->aParam[5];

    p->pUser = (ਖਾਲੀ *)pCube;
    p->xDelUser = cube_context_free;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( nCoord==6 );
  *piRes = 0;
  ਜੇ( aCoord[0]<=(pCube->x+pCube->ਚੌੜਾਈ)
   && aCoord[1]>=pCube->x
   && aCoord[2]<=(pCube->y+pCube->ਉਚਾਈ)
   && aCoord[3]>=pCube->y
   && aCoord[4]<=(pCube->z+pCube->ਡੂੰਘਾਈ)
   && aCoord[5]>=pCube->z
  ){
    *piRes = 1;
  }

  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_RTREE */

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI ਰਜਿਸਟਰ_cube_geom(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_RTREE
  ਅਣਵਰਤਿਆ_PARAMETER(client_ਅੰਕੜੇ);
  ਅਣਵਰਤਿਆ_PARAMETER(interp);
  ਅਣਵਰਤਿਆ_PARAMETER(objc);
  ਅਣਵਰਤਿਆ_PARAMETER(objv);
#ਨਹੀਂਤਾਂ
  ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp*, ਅਚਲ ਅੱਖਰ*, sqlite3**);
  ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);
  sqlite3 *db;
  ਪੂਰਨ rc;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "cube", cube_geom, (ਖਾਲੀ *)&g_ਇੱਥੇ);
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_STATIC);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI ਰਜਿਸਟਰ_circle_geom(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_RTREE
  ਅਣਵਰਤਿਆ_PARAMETER(client_ਅੰਕੜੇ);
  ਅਣਵਰਤਿਆ_PARAMETER(interp);
  ਅਣਵਰਤਿਆ_PARAMETER(objc);
  ਅਣਵਰਤਿਆ_PARAMETER(objv);
#ਨਹੀਂਤਾਂ
  ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp*, ਅਚਲ ਅੱਖਰ*, sqlite3**);
  ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);
  sqlite3 *db;
  ਪੂਰਨ rc;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "circle", circle_geom, 0);
  ਜੇ( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "Qcircle",
                                      circle_query_func, 0, 0);
  }
  ਜੇ( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "breadthfirstsearch",
                                      bfs_query_func, 0, 0);
  }
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_STATIC);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}

ਪੂਰਨ Sqlitetestrtree_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_cube_geom", ਰਜਿਸਟਰ_cube_geom, 0, 0);
  Tcl_CreateObjCommand(interp, "register_circle_geom",ਰਜਿਸਟਰ_circle_geom,0,0);
  ਵਾਪਸ TCL_OK;
}
