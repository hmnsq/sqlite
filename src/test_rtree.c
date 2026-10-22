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

#అన్తర్భావ "sqlite3.h"
#అన్తర్భావ "tclsqlite.h"

/* Solely for the UNUSED_PARAMETER() macro. */
#అన్తర్భావ "sqliteInt.h"

#నిర్వచితమైతే SQLITE_ENABLE_RTREE
/* 
** Type used to cache parameter information for the "circle" r-tree geometry
** callback.
*/
రకనిర్వచనం నిర్మాణం Circle Circle;
నిర్మాణం Circle {
  నిర్మాణం Box {
    ద్విగుణ xmin;
    ద్విగుణ xmax;
    ద్విగుణ ymin;
    ద్విగుణ ymax;
  } aBox[2];
  ద్విగుణ centerx;
  ద్విగుణ centery;
  ద్విగుణ radius;
  ద్విగుణ mxArea;
  పూర్ణాంకం eScoreType;
};

/*
** Destructor function for Circle objects allocated by circle_geom().
*/
స్థిర శూన్య circle_del(శూన్య *p){
  sqlite3_free(p);
}

/*
** Implementation of "circle" r-tree geometry callback.
*/
స్థిర పూర్ణాంకం circle_geom(
  sqlite3_rtree_geometry *p,
  పూర్ణాంకం nCoord, 
  sqlite3_rtree_dbl *aCoord,
  పూర్ణాంకం *pRes
){
  పూర్ణాంకం i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  ద్విగుణ xmin, xmax;              /* X dimensions of box being tested */
  ద్విగుణ ymin, ymax;              /* X dimensions of box being tested */

  xmin = aCoord[0];
  xmax = aCoord[1];
  ymin = aCoord[2];
  ymax = aCoord[3];
  pCircle = (Circle *)p->pUser;
  ఒకవేళ( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    ఒకవేళ( nCoord!=4 ) తిరిగి SQLITE_ERROR;

    /* Test that the correct number of parameters (3) have been supplied,
    ** and that the parameters are in range (that the radius of the circle 
    ** radius is greater than zero). */
    ఒకవేళ( p->nParam!=3 || p->aParam[2]<0.0 ) తిరిగి SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(పరిమాణం(Circle)));
    ఒకవేళ( !pCircle ) తిరిగి SQLITE_NOMEM;
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
  కొరకు(i=0; i<4; i++){
    ద్విగుణ x = (i&0x01) ? xmax : xmin;
    ద్విగుణ y = (i&0x02) ? ymax : ymin;
    ద్విగుణ d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    ఒకవేళ( d2<(pCircle->radius*pCircle->radius) ){
      *pRes = 1;
      తిరిగి SQLITE_OK;
    }
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  కొరకు(i=0; i<2; i++){
    ఒకవేళ( xmin<=pCircle->aBox[i].xmin 
     && xmax>=pCircle->aBox[i].xmax 
     && ymin<=pCircle->aBox[i].ymin 
     && ymax>=pCircle->aBox[i].ymax 
    ){
      *pRes = 1;
      తిరిగి SQLITE_OK;
    }
  }

  /* The specified bounding box does not intersect the circular region. Set
  ** the output variable to zero and return SQLITE_OK. */
  *pRes = 0;
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం circle_query_func(sqlite3_rtree_query_info *p){
  పూర్ణాంకం i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  ద్విగుణ xmin, xmax;              /* X dimensions of box being tested */
  ద్విగుణ ymin, ymax;              /* X dimensions of box being tested */
  పూర్ణాంకం nWithin = 0;                /* Number of corners inside the circle */

  xmin = p->aCoord[0];
  xmax = p->aCoord[1];
  ymin = p->aCoord[2];
  ymax = p->aCoord[3];
  pCircle = (Circle *)p->pUser;
  ఒకవేళ( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    ఒకవేళ( p->nCoord!=4 ) తిరిగి SQLITE_ERROR;

    /* Test that the correct number of parameters (1 or 4) have been supplied.
    */
    ఒకవేళ( p->nParam!=4 && p->nParam!=1 ) తిరిగి SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(పరిమాణం(Circle)));
    ఒకవేళ( !pCircle ) తిరిగి SQLITE_NOMEM;
    p->xDelUser = circle_del;

    /* Record the center and radius of the circular region. One way that
    ** tested bounding boxes that intersect the circular region are detected
    ** is by testing if each corner of the bounding box lies within radius
    ** units of the center of the circle. */
    ఒకవేళ( p->nParam==4 ){
      pCircle->centerx = p->aParam[0];
      pCircle->centery = p->aParam[1];
      pCircle->radius = p->aParam[2];
      pCircle->eScoreType = (పూర్ణాంకం)p->aParam[3];
    }లేకపోతే{
      స్థిరరాశి అక్షరం *z = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(p->apSqlParam[0]);
      pCircle->centerx = 0.0;
      pCircle->centery = 0.0;
      pCircle->radius = 0.0;
      pCircle->eScoreType = 0;
      అయితే( z && z[0] ){
        ఒకవేళ( z[0]=='r' && z[1]==':' ){
          pCircle->radius = atof(&z[2]);
        }లేకపోతే ఒకవేళ( z[0]=='x' && z[1]==':' ){
          pCircle->centerx = atof(&z[2]);
        }లేకపోతే ఒకవేళ( z[0]=='y' && z[1]==':' ){
          pCircle->centery = atof(&z[2]);
        }లేకపోతే ఒకవేళ( z[0]=='e' && z[1]==':' ){
          pCircle->eScoreType = (పూర్ణాంకం)atof(&z[2]);
        }లేకపోతే ఒకవేళ( z[0]==' ' ){
          z++;
          కొనసాగు;
        }
        అయితే( z[0]!=0 && z[0]!=' ' ) z++;
        అయితే( z[0]==' ' ) z++;
      }
    }
    ఒకవేళ( pCircle->radius<0.0 ){
      sqlite3_free(pCircle);
      తిరిగి SQLITE_NOMEM;
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
  కొరకు(i=0; i<4; i++){
    ద్విగుణ x = (i&0x01) ? xmax : xmin;
    ద్విగుణ y = (i&0x02) ? ymax : ymin;
    ద్విగుణ d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    ఒకవేళ( d2<(pCircle->radius*pCircle->radius) ) nWithin++;
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  ఒకవేళ( nWithin==0 ){
    కొరకు(i=0; i<2; i++){
      ఒకవేళ( xmin<=pCircle->aBox[i].xmin 
       && xmax>=pCircle->aBox[i].xmax 
       && ymin<=pCircle->aBox[i].ymin 
       && ymax>=pCircle->aBox[i].ymax 
      ){
        nWithin = 1;
        ఆపు;
      }
    }
  }

  ఒకవేళ( pCircle->eScoreType==1 ){
    /* Depth first search */
    p->rScore = p->iLevel;
  }లేకపోతే ఒకవేళ( pCircle->eScoreType==2 ){
    /* Breadth first search */
    p->rScore = 100 - p->iLevel;
  }లేకపోతే ఒకవేళ( pCircle->eScoreType==3 ){
    /* Depth-first search, except sort the leaf nodes by area with
    ** the largest area first */
    ఒకవేళ( p->iLevel==1 ){
      p->rScore = 1.0 - (xmax-xmin)*(ymax-ymin)/pCircle->mxArea;
      ఒకవేళ( p->rScore<0.01 ) p->rScore = 0.01;
    }లేకపోతే{
      p->rScore = 0.0;
    }
  }లేకపోతే ఒకవేళ( pCircle->eScoreType==4 ){
    /* Depth-first search, except exclude odd rowids */
    p->rScore = p->iLevel;
    ఒకవేళ( p->iRowid&1 ) nWithin = 0;
  }లేకపోతే{
    /* Breadth-first search, except exclude odd rowids */
    p->rScore = 100 - p->iLevel;
    ఒకవేళ( p->iRowid&1 ) nWithin = 0;
  }
  ఒకవేళ( nWithin==0 ){
    p->eWithin = NOT_WITHIN;
  }లేకపోతే ఒకవేళ( nWithin>=4 ){
    p->eWithin = FULLY_WITHIN;
  }లేకపోతే{
    p->eWithin = PARTLY_WITHIN;
  }
  తిరిగి SQLITE_OK;
}
/*
** Implementation of "breadthfirstsearch" r-tree geometry callback using the 
** 2nd-generation interface that allows scoring.
**
**     ... WHERE id MATCH breadthfirstsearch($x0,$x1,$y0,$y1) ...
**
** It returns all entries whose bounding boxes overlap with $x0,$x1,$y0,$y1.
*/
స్థిర పూర్ణాంకం bfs_query_func(sqlite3_rtree_query_info *p){
  ద్విగుణ x0,x1,y0,y1;        /* Dimensions of box being tested */
  ద్విగుణ bx0,bx1,by0,by1;    /* Boundary of the query function */

  ఒకవేళ( p->nParam!=4 ) తిరిగి SQLITE_ERROR;
  x0 = p->aCoord[0];
  x1 = p->aCoord[1];
  y0 = p->aCoord[2];
  y1 = p->aCoord[3];
  bx0 = p->aParam[0];
  bx1 = p->aParam[1];
  by0 = p->aParam[2];
  by1 = p->aParam[3];
  p->rScore = 100 - p->iLevel;
  ఒకవేళ( p->e_మాతృ_Within==FULLY_WITHIN ){
    p->eWithin = FULLY_WITHIN;
  }లేకపోతే ఒకవేళ( x0>=bx0 && x1<=bx1 && y0>=by0 && y1<=by1 ){
    p->eWithin = FULLY_WITHIN;
  }లేకపోతే ఒకవేళ( x1>=bx0 && x0<=bx1 && y1>=by0 && y0<=by1 ){
    p->eWithin = PARTLY_WITHIN;
  }లేకపోతే{
    p->eWithin = NOT_WITHIN;
  }
  తిరిగి SQLITE_OK;
}

/* END of implementation of "circle" geometry callback.
**************************************************************************
*************************************************************************/

#అన్తర్భావ <assert.h>
#అన్తర్భావ "tclsqlite.h"

రకనిర్వచనం నిర్మాణం Cube Cube;
నిర్మాణం Cube {
  ద్విగుణ x;
  ద్విగుణ y;
  ద్విగుణ z;
  ద్విగుణ వెడల్పు;
  ద్విగుణ ఎత్తు;
  ద్విగుణ లోతు;
};

స్థిర శూన్య cube_context_free(శూన్య *p){
  sqlite3_free(p);
}

/*
** The context pointer registered along with the 'cube' callback is
** always ((void *)&gHere). This is just to facilitate testing, it is not
** actually used for anything.
*/
స్థిర పూర్ణాంకం g_ఇక్కడ = 42;

/*
** Implementation of a simple r-tree geom callback to test for intersection
** of r-tree rows with a "cube" shape. Cubes are defined by six scalar
** coordinates as follows:
**
**   cube(x, y, z, width, height, depth)
**
** The width, height and depth parameters must all be greater than zero.
*/
స్థిర పూర్ణాంకం cube_geom(
  sqlite3_rtree_geometry *p,
  పూర్ణాంకం nCoord,
  sqlite3_rtree_dbl *aCoord,
  పూర్ణాంకం *piRes
){
  Cube *pCube = (Cube *)p->pUser;

  ప్రతిజ్ఞా( p->pContext==(శూన్య *)&g_ఇక్కడ );

  ఒకవేళ( pCube==0 ){
    ఒకవేళ( p->nParam!=6 || nCoord!=6
     || p->aParam[3]<=0.0 || p->aParam[4]<=0.0 || p->aParam[5]<=0.0
    ){
      తిరిగి SQLITE_ERROR;
    }
    pCube = (Cube *)sqlite3_malloc(పరిమాణం(Cube));
    ఒకవేళ( !pCube ){
      తిరిగి SQLITE_NOMEM;
    }
    pCube->x = p->aParam[0];
    pCube->y = p->aParam[1];
    pCube->z = p->aParam[2];
    pCube->వెడల్పు = p->aParam[3];
    pCube->ఎత్తు = p->aParam[4];
    pCube->లోతు = p->aParam[5];

    p->pUser = (శూన్య *)pCube;
    p->xDelUser = cube_context_free;
  }

  ప్రతిజ్ఞా( nCoord==6 );
  *piRes = 0;
  ఒకవేళ( aCoord[0]<=(pCube->x+pCube->వెడల్పు)
   && aCoord[1]>=pCube->x
   && aCoord[2]<=(pCube->y+pCube->ఎత్తు)
   && aCoord[3]>=pCube->y
   && aCoord[4]<=(pCube->z+pCube->లోతు)
   && aCoord[5]>=pCube->z
  ){
    *piRes = 1;
  }

  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_RTREE */

స్థిర పూర్ణాంకం SQLITE_TCLAPI రిజిస్టరు_cube_geom(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
#నిర్వచితం_కాకపోతే SQLITE_ENABLE_RTREE
  వాడని_PARAMETER(client_దత్తాంశం);
  వాడని_PARAMETER(interp);
  వాడని_PARAMETER(objc);
  వాడని_PARAMETER(objv);
#లేకపోతే
  బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp*, స్థిరరాశి అక్షరం*, sqlite3**);
  బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);
  sqlite3 *db;
  పూర్ణాంకం rc;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "cube", cube_geom, (శూన్య *)&g_ఇక్కడ);
  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_STATIC);
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI రిజిస్టరు_circle_geom(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
#నిర్వచితం_కాకపోతే SQLITE_ENABLE_RTREE
  వాడని_PARAMETER(client_దత్తాంశం);
  వాడని_PARAMETER(interp);
  వాడని_PARAMETER(objc);
  వాడని_PARAMETER(objv);
#లేకపోతే
  బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp*, స్థిరరాశి అక్షరం*, sqlite3**);
  బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);
  sqlite3 *db;
  పూర్ణాంకం rc;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "circle", circle_geom, 0);
  ఒకవేళ( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "Qcircle",
                                      circle_query_func, 0, 0);
  }
  ఒకవేళ( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "breadthfirstsearch",
                                      bfs_query_func, 0, 0);
  }
  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_STATIC);
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}

పూర్ణాంకం Sqlitetestrtree_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_cube_geom", రిజిస్టరు_cube_geom, 0, 0);
  Tcl_CreateObjCommand(interp, "register_circle_geom",రిజిస్టరు_circle_geom,0,0);
  తిరిగి TCL_OK;
}
