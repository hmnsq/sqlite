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

#அந்தர்பாவ "sqlite3.h"
#அந்தர்பாவ "tclsqlite.h"

/* Solely for the UNUSED_PARAMETER() macro. */
#அந்தர்பாவ "sqliteInt.h"

#ifdef SQLITE_ENABLE_RTREE
/* 
** Type used to cache parameter information for the "circle" r-tree geometry
** callback.
*/
வகைவரை கட்டமைப்பு Circle Circle;
கட்டமைப்பு Circle {
  கட்டமைப்பு Box {
    இரட்டை xmin;
    இரட்டை xmax;
    இரட்டை ymin;
    இரட்டை ymax;
  } aBox[2];
  இரட்டை centerx;
  இரட்டை centery;
  இரட்டை radius;
  இரட்டை mxArea;
  முழு eScoreType;
};

/*
** Destructor function for Circle objects allocated by circle_geom().
*/
நிலையான வெறுமை circle_del(வெறுமை *p){
  sqlite3_free(p);
}

/*
** Implementation of "circle" r-tree geometry callback.
*/
நிலையான முழு circle_geom(
  sqlite3_rtree_geometry *p,
  முழு nCoord, 
  sqlite3_rtree_dbl *aCoord,
  முழு *pRes
){
  முழு i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  இரட்டை xmin, xmax;              /* X dimensions of box being tested */
  இரட்டை ymin, ymax;              /* X dimensions of box being tested */

  xmin = aCoord[0];
  xmax = aCoord[1];
  ymin = aCoord[2];
  ymax = aCoord[3];
  pCircle = (Circle *)p->pUser;
  எனில்( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    எனில்( nCoord!=4 ) திரும்பு SQLITE_ERROR;

    /* Test that the correct number of parameters (3) have been supplied,
    ** and that the parameters are in range (that the radius of the circle 
    ** radius is greater than zero). */
    எனில்( p->nParam!=3 || p->aParam[2]<0.0 ) திரும்பு SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(அளவு(Circle)));
    எனில்( !pCircle ) திரும்பு SQLITE_NOMEM;
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
  க்கு(i=0; i<4; i++){
    இரட்டை x = (i&0x01) ? xmax : xmin;
    இரட்டை y = (i&0x02) ? ymax : ymin;
    இரட்டை d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    எனில்( d2<(pCircle->radius*pCircle->radius) ){
      *pRes = 1;
      திரும்பு SQLITE_OK;
    }
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  க்கு(i=0; i<2; i++){
    எனில்( xmin<=pCircle->aBox[i].xmin 
     && xmax>=pCircle->aBox[i].xmax 
     && ymin<=pCircle->aBox[i].ymin 
     && ymax>=pCircle->aBox[i].ymax 
    ){
      *pRes = 1;
      திரும்பு SQLITE_OK;
    }
  }

  /* The specified bounding box does not intersect the circular region. Set
  ** the output variable to zero and return SQLITE_OK. */
  *pRes = 0;
  திரும்பு SQLITE_OK;
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
நிலையான முழு circle_query_func(sqlite3_rtree_query_info *p){
  முழு i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  இரட்டை xmin, xmax;              /* X dimensions of box being tested */
  இரட்டை ymin, ymax;              /* X dimensions of box being tested */
  முழு nWithin = 0;                /* Number of corners inside the circle */

  xmin = p->aCoord[0];
  xmax = p->aCoord[1];
  ymin = p->aCoord[2];
  ymax = p->aCoord[3];
  pCircle = (Circle *)p->pUser;
  எனில்( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    எனில்( p->nCoord!=4 ) திரும்பு SQLITE_ERROR;

    /* Test that the correct number of parameters (1 or 4) have been supplied.
    */
    எனில்( p->nParam!=4 && p->nParam!=1 ) திரும்பு SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(அளவு(Circle)));
    எனில்( !pCircle ) திரும்பு SQLITE_NOMEM;
    p->xDelUser = circle_del;

    /* Record the center and radius of the circular region. One way that
    ** tested bounding boxes that intersect the circular region are detected
    ** is by testing if each corner of the bounding box lies within radius
    ** units of the center of the circle. */
    எனில்( p->nParam==4 ){
      pCircle->centerx = p->aParam[0];
      pCircle->centery = p->aParam[1];
      pCircle->radius = p->aParam[2];
      pCircle->eScoreType = (முழு)p->aParam[3];
    }இல்லையெனில்{
      மாறிலி எழுத்து *z = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(p->apSqlParam[0]);
      pCircle->centerx = 0.0;
      pCircle->centery = 0.0;
      pCircle->radius = 0.0;
      pCircle->eScoreType = 0;
      போது( z && z[0] ){
        எனில்( z[0]=='r' && z[1]==':' ){
          pCircle->radius = atof(&z[2]);
        }இல்லையெனில் எனில்( z[0]=='x' && z[1]==':' ){
          pCircle->centerx = atof(&z[2]);
        }இல்லையெனில் எனில்( z[0]=='y' && z[1]==':' ){
          pCircle->centery = atof(&z[2]);
        }இல்லையெனில் எனில்( z[0]=='e' && z[1]==':' ){
          pCircle->eScoreType = (முழு)atof(&z[2]);
        }இல்லையெனில் எனில்( z[0]==' ' ){
          z++;
          தொடர்;
        }
        போது( z[0]!=0 && z[0]!=' ' ) z++;
        போது( z[0]==' ' ) z++;
      }
    }
    எனில்( pCircle->radius<0.0 ){
      sqlite3_free(pCircle);
      திரும்பு SQLITE_NOMEM;
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
  க்கு(i=0; i<4; i++){
    இரட்டை x = (i&0x01) ? xmax : xmin;
    இரட்டை y = (i&0x02) ? ymax : ymin;
    இரட்டை d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    எனில்( d2<(pCircle->radius*pCircle->radius) ) nWithin++;
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  எனில்( nWithin==0 ){
    க்கு(i=0; i<2; i++){
      எனில்( xmin<=pCircle->aBox[i].xmin 
       && xmax>=pCircle->aBox[i].xmax 
       && ymin<=pCircle->aBox[i].ymin 
       && ymax>=pCircle->aBox[i].ymax 
      ){
        nWithin = 1;
        நிறுத்து;
      }
    }
  }

  எனில்( pCircle->eScoreType==1 ){
    /* Depth first search */
    p->rScore = p->iLevel;
  }இல்லையெனில் எனில்( pCircle->eScoreType==2 ){
    /* Breadth first search */
    p->rScore = 100 - p->iLevel;
  }இல்லையெனில் எனில்( pCircle->eScoreType==3 ){
    /* Depth-first search, except sort the leaf nodes by area with
    ** the largest area first */
    எனில்( p->iLevel==1 ){
      p->rScore = 1.0 - (xmax-xmin)*(ymax-ymin)/pCircle->mxArea;
      எனில்( p->rScore<0.01 ) p->rScore = 0.01;
    }இல்லையெனில்{
      p->rScore = 0.0;
    }
  }இல்லையெனில் எனில்( pCircle->eScoreType==4 ){
    /* Depth-first search, except exclude odd rowids */
    p->rScore = p->iLevel;
    எனில்( p->iRowid&1 ) nWithin = 0;
  }இல்லையெனில்{
    /* Breadth-first search, except exclude odd rowids */
    p->rScore = 100 - p->iLevel;
    எனில்( p->iRowid&1 ) nWithin = 0;
  }
  எனில்( nWithin==0 ){
    p->eWithin = NOT_WITHIN;
  }இல்லையெனில் எனில்( nWithin>=4 ){
    p->eWithin = FULLY_WITHIN;
  }இல்லையெனில்{
    p->eWithin = PARTLY_WITHIN;
  }
  திரும்பு SQLITE_OK;
}
/*
** Implementation of "breadthfirstsearch" r-tree geometry callback using the 
** 2nd-generation interface that allows scoring.
**
**     ... WHERE id MATCH breadthfirstsearch($x0,$x1,$y0,$y1) ...
**
** It returns all entries whose bounding boxes overlap with $x0,$x1,$y0,$y1.
*/
நிலையான முழு bfs_query_func(sqlite3_rtree_query_info *p){
  இரட்டை x0,x1,y0,y1;        /* Dimensions of box being tested */
  இரட்டை bx0,bx1,by0,by1;    /* Boundary of the query function */

  எனில்( p->nParam!=4 ) திரும்பு SQLITE_ERROR;
  x0 = p->aCoord[0];
  x1 = p->aCoord[1];
  y0 = p->aCoord[2];
  y1 = p->aCoord[3];
  bx0 = p->aParam[0];
  bx1 = p->aParam[1];
  by0 = p->aParam[2];
  by1 = p->aParam[3];
  p->rScore = 100 - p->iLevel;
  எனில்( p->eParentWithin==FULLY_WITHIN ){
    p->eWithin = FULLY_WITHIN;
  }இல்லையெனில் எனில்( x0>=bx0 && x1<=bx1 && y0>=by0 && y1<=by1 ){
    p->eWithin = FULLY_WITHIN;
  }இல்லையெனில் எனில்( x1>=bx0 && x0<=bx1 && y1>=by0 && y0<=by1 ){
    p->eWithin = PARTLY_WITHIN;
  }இல்லையெனில்{
    p->eWithin = NOT_WITHIN;
  }
  திரும்பு SQLITE_OK;
}

/* END of implementation of "circle" geometry callback.
**************************************************************************
*************************************************************************/

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ "tclsqlite.h"

வகைவரை கட்டமைப்பு Cube Cube;
கட்டமைப்பு Cube {
  இரட்டை x;
  இரட்டை y;
  இரட்டை z;
  இரட்டை width;
  இரட்டை height;
  இரட்டை depth;
};

நிலையான வெறுமை cube_context_free(வெறுமை *p){
  sqlite3_free(p);
}

/*
** The context pointer registered along with the 'cube' callback is
** always ((void *)&gHere). This is just to facilitate testing, it is not
** actually used for anything.
*/
நிலையான முழு gHere = 42;

/*
** Implementation of a simple r-tree geom callback to test for intersection
** of r-tree rows with a "cube" shape. Cubes are defined by six scalar
** coordinates as follows:
**
**   cube(x, y, z, width, height, depth)
**
** The width, height and depth parameters must all be greater than zero.
*/
நிலையான முழு cube_geom(
  sqlite3_rtree_geometry *p,
  முழு nCoord,
  sqlite3_rtree_dbl *aCoord,
  முழு *piRes
){
  Cube *pCube = (Cube *)p->pUser;

  ப்ரதிஜ்ஞா( p->pContext==(வெறுமை *)&gHere );

  எனில்( pCube==0 ){
    எனில்( p->nParam!=6 || nCoord!=6
     || p->aParam[3]<=0.0 || p->aParam[4]<=0.0 || p->aParam[5]<=0.0
    ){
      திரும்பு SQLITE_ERROR;
    }
    pCube = (Cube *)sqlite3_malloc(அளவு(Cube));
    எனில்( !pCube ){
      திரும்பு SQLITE_NOMEM;
    }
    pCube->x = p->aParam[0];
    pCube->y = p->aParam[1];
    pCube->z = p->aParam[2];
    pCube->width = p->aParam[3];
    pCube->height = p->aParam[4];
    pCube->depth = p->aParam[5];

    p->pUser = (வெறுமை *)pCube;
    p->xDelUser = cube_context_free;
  }

  ப்ரதிஜ்ஞா( nCoord==6 );
  *piRes = 0;
  எனில்( aCoord[0]<=(pCube->x+pCube->width)
   && aCoord[1]>=pCube->x
   && aCoord[2]<=(pCube->y+pCube->height)
   && aCoord[3]>=pCube->y
   && aCoord[4]<=(pCube->z+pCube->depth)
   && aCoord[5]>=pCube->z
  ){
    *piRes = 1;
  }

  திரும்பு SQLITE_OK;
}
#endif /* SQLITE_ENABLE_RTREE */

நிலையான முழு SQLITE_TCLAPI register_cube_geom(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_ENABLE_RTREE
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(interp);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
#இல்லையெனில்
  வெளி முழு getDbPointer(Tcl_Interp*, மாறிலி எழுத்து*, sqlite3**);
  வெளி மாறிலி எழுத்து *sqlite3ErrName(முழு);
  sqlite3 *db;
  முழு rc;

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DB");
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "cube", cube_geom, (வெறுமை *)&gHere);
  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_STATIC);
#endif
  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI register_circle_geom(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_ENABLE_RTREE
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(interp);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
#இல்லையெனில்
  வெளி முழு getDbPointer(Tcl_Interp*, மாறிலி எழுத்து*, sqlite3**);
  வெளி மாறிலி எழுத்து *sqlite3ErrName(முழு);
  sqlite3 *db;
  முழு rc;

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DB");
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "circle", circle_geom, 0);
  எனில்( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "Qcircle",
                                      circle_query_func, 0, 0);
  }
  எனில்( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "breadthfirstsearch",
                                      bfs_query_func, 0, 0);
  }
  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_STATIC);
#endif
  திரும்பு TCL_OK;
}

முழு Sqlitetestrtree_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_cube_geom", register_cube_geom, 0, 0);
  Tcl_CreateObjCommand(interp, "register_circle_geom",register_circle_geom,0,0);
  திரும்பு TCL_OK;
}
