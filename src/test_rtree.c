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

#شامل "sqlite3.h"
#شامل "tclsqlite.h"

/* Solely for the UNUSED_PARAMETER() macro. */
#شامل "sqliteInt.h"

#اگر_تعریف_شدہ SQLITE_ENABLE_RTREE
/* 
** Type used to cache parameter information for the "circle" r-tree geometry
** callback.
*/
قسمتعریف ساخت Circle Circle;
ساخت Circle {
  ساخت Box {
    دوہرا xmin;
    دوہرا xmax;
    دوہرا ymin;
    دوہرا ymax;
  } aBox[2];
  دوہرا centerx;
  دوہرا centery;
  دوہرا radius;
  دوہرا mxArea;
  صحیح eScoreType;
};

/*
** Destructor function for Circle objects allocated by circle_geom().
*/
ساکن خالی circle_del(خالی *p){
  sqlite3_free(p);
}

/*
** Implementation of "circle" r-tree geometry callback.
*/
ساکن صحیح circle_geom(
  sqlite3_rtree_geometry *p,
  صحیح nCoord, 
  sqlite3_rtree_dbl *aCoord,
  صحیح *pRes
){
  صحیح i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  دوہرا xmin, xmax;              /* X dimensions of box being tested */
  دوہرا ymin, ymax;              /* X dimensions of box being tested */

  xmin = aCoord[0];
  xmax = aCoord[1];
  ymin = aCoord[2];
  ymax = aCoord[3];
  pCircle = (Circle *)p->pUser;
  اگر( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    اگر( nCoord!=4 ) واپس SQLITE_ERROR;

    /* Test that the correct number of parameters (3) have been supplied,
    ** and that the parameters are in range (that the radius of the circle 
    ** radius is greater than zero). */
    اگر( p->nParam!=3 || p->aParam[2]<0.0 ) واپس SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(حجم(Circle)));
    اگر( !pCircle ) واپس SQLITE_NOMEM;
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
  برائے(i=0; i<4; i++){
    دوہرا x = (i&0x01) ? xmax : xmin;
    دوہرا y = (i&0x02) ? ymax : ymin;
    دوہرا d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    اگر( d2<(pCircle->radius*pCircle->radius) ){
      *pRes = 1;
      واپس SQLITE_OK;
    }
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  برائے(i=0; i<2; i++){
    اگر( xmin<=pCircle->aBox[i].xmin 
     && xmax>=pCircle->aBox[i].xmax 
     && ymin<=pCircle->aBox[i].ymin 
     && ymax>=pCircle->aBox[i].ymax 
    ){
      *pRes = 1;
      واپس SQLITE_OK;
    }
  }

  /* The specified bounding box does not intersect the circular region. Set
  ** the output variable to zero and return SQLITE_OK. */
  *pRes = 0;
  واپس SQLITE_OK;
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
ساکن صحیح circle_query_func(sqlite3_rtree_query_info *p){
  صحیح i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  دوہرا xmin, xmax;              /* X dimensions of box being tested */
  دوہرا ymin, ymax;              /* X dimensions of box being tested */
  صحیح nWithin = 0;                /* Number of corners inside the circle */

  xmin = p->aCoord[0];
  xmax = p->aCoord[1];
  ymin = p->aCoord[2];
  ymax = p->aCoord[3];
  pCircle = (Circle *)p->pUser;
  اگر( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    اگر( p->nCoord!=4 ) واپس SQLITE_ERROR;

    /* Test that the correct number of parameters (1 or 4) have been supplied.
    */
    اگر( p->nParam!=4 && p->nParam!=1 ) واپس SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(حجم(Circle)));
    اگر( !pCircle ) واپس SQLITE_NOMEM;
    p->xDelUser = circle_del;

    /* Record the center and radius of the circular region. One way that
    ** tested bounding boxes that intersect the circular region are detected
    ** is by testing if each corner of the bounding box lies within radius
    ** units of the center of the circle. */
    اگر( p->nParam==4 ){
      pCircle->centerx = p->aParam[0];
      pCircle->centery = p->aParam[1];
      pCircle->radius = p->aParam[2];
      pCircle->eScoreType = (صحیح)p->aParam[3];
    }ورنہ{
      ثابت حرف *z = (ثابت حرف*)sqlite3_قدر_متن(p->apSqlParam[0]);
      pCircle->centerx = 0.0;
      pCircle->centery = 0.0;
      pCircle->radius = 0.0;
      pCircle->eScoreType = 0;
      جب( z && z[0] ){
        اگر( z[0]=='r' && z[1]==':' ){
          pCircle->radius = atof(&z[2]);
        }ورنہ اگر( z[0]=='x' && z[1]==':' ){
          pCircle->centerx = atof(&z[2]);
        }ورنہ اگر( z[0]=='y' && z[1]==':' ){
          pCircle->centery = atof(&z[2]);
        }ورنہ اگر( z[0]=='e' && z[1]==':' ){
          pCircle->eScoreType = (صحیح)atof(&z[2]);
        }ورنہ اگر( z[0]==' ' ){
          z++;
          جاری;
        }
        جب( z[0]!=0 && z[0]!=' ' ) z++;
        جب( z[0]==' ' ) z++;
      }
    }
    اگر( pCircle->radius<0.0 ){
      sqlite3_free(pCircle);
      واپس SQLITE_NOMEM;
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
  برائے(i=0; i<4; i++){
    دوہرا x = (i&0x01) ? xmax : xmin;
    دوہرا y = (i&0x02) ? ymax : ymin;
    دوہرا d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    اگر( d2<(pCircle->radius*pCircle->radius) ) nWithin++;
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  اگر( nWithin==0 ){
    برائے(i=0; i<2; i++){
      اگر( xmin<=pCircle->aBox[i].xmin 
       && xmax>=pCircle->aBox[i].xmax 
       && ymin<=pCircle->aBox[i].ymin 
       && ymax>=pCircle->aBox[i].ymax 
      ){
        nWithin = 1;
        توڑو;
      }
    }
  }

  اگر( pCircle->eScoreType==1 ){
    /* Depth first search */
    p->rScore = p->iLevel;
  }ورنہ اگر( pCircle->eScoreType==2 ){
    /* Breadth first search */
    p->rScore = 100 - p->iLevel;
  }ورنہ اگر( pCircle->eScoreType==3 ){
    /* Depth-first search, except sort the leaf nodes by area with
    ** the largest area first */
    اگر( p->iLevel==1 ){
      p->rScore = 1.0 - (xmax-xmin)*(ymax-ymin)/pCircle->mxArea;
      اگر( p->rScore<0.01 ) p->rScore = 0.01;
    }ورنہ{
      p->rScore = 0.0;
    }
  }ورنہ اگر( pCircle->eScoreType==4 ){
    /* Depth-first search, except exclude odd rowids */
    p->rScore = p->iLevel;
    اگر( p->iRowid&1 ) nWithin = 0;
  }ورنہ{
    /* Breadth-first search, except exclude odd rowids */
    p->rScore = 100 - p->iLevel;
    اگر( p->iRowid&1 ) nWithin = 0;
  }
  اگر( nWithin==0 ){
    p->eWithin = NOT_WITHIN;
  }ورنہ اگر( nWithin>=4 ){
    p->eWithin = FULLY_WITHIN;
  }ورنہ{
    p->eWithin = PARTLY_WITHIN;
  }
  واپس SQLITE_OK;
}
/*
** Implementation of "breadthfirstsearch" r-tree geometry callback using the 
** 2nd-generation interface that allows scoring.
**
**     ... WHERE id MATCH breadthfirstsearch($x0,$x1,$y0,$y1) ...
**
** It returns all entries whose bounding boxes overlap with $x0,$x1,$y0,$y1.
*/
ساکن صحیح bfs_query_func(sqlite3_rtree_query_info *p){
  دوہرا x0,x1,y0,y1;        /* Dimensions of box being tested */
  دوہرا bx0,bx1,by0,by1;    /* Boundary of the query function */

  اگر( p->nParam!=4 ) واپس SQLITE_ERROR;
  x0 = p->aCoord[0];
  x1 = p->aCoord[1];
  y0 = p->aCoord[2];
  y1 = p->aCoord[3];
  bx0 = p->aParam[0];
  bx1 = p->aParam[1];
  by0 = p->aParam[2];
  by1 = p->aParam[3];
  p->rScore = 100 - p->iLevel;
  اگر( p->e_والد_Within==FULLY_WITHIN ){
    p->eWithin = FULLY_WITHIN;
  }ورنہ اگر( x0>=bx0 && x1<=bx1 && y0>=by0 && y1<=by1 ){
    p->eWithin = FULLY_WITHIN;
  }ورنہ اگر( x1>=bx0 && x0<=bx1 && y1>=by0 && y0<=by1 ){
    p->eWithin = PARTLY_WITHIN;
  }ورنہ{
    p->eWithin = NOT_WITHIN;
  }
  واپس SQLITE_OK;
}

/* END of implementation of "circle" geometry callback.
**************************************************************************
*************************************************************************/

#شامل <assert.h>
#شامل "tclsqlite.h"

قسمتعریف ساخت Cube Cube;
ساخت Cube {
  دوہرا x;
  دوہرا y;
  دوہرا z;
  دوہرا چوڑائی;
  دوہرا اونچائی;
  دوہرا گہرائی;
};

ساکن خالی cube_context_free(خالی *p){
  sqlite3_free(p);
}

/*
** The context pointer registered along with the 'cube' callback is
** always ((void *)&gHere). This is just to facilitate testing, it is not
** actually used for anything.
*/
ساکن صحیح g_یہاں = 42;

/*
** Implementation of a simple r-tree geom callback to test for intersection
** of r-tree rows with a "cube" shape. Cubes are defined by six scalar
** coordinates as follows:
**
**   cube(x, y, z, width, height, depth)
**
** The width, height and depth parameters must all be greater than zero.
*/
ساکن صحیح cube_geom(
  sqlite3_rtree_geometry *p,
  صحیح nCoord,
  sqlite3_rtree_dbl *aCoord,
  صحیح *piRes
){
  Cube *pCube = (Cube *)p->pUser;

  أكد( p->pContext==(خالی *)&g_یہاں );

  اگر( pCube==0 ){
    اگر( p->nParam!=6 || nCoord!=6
     || p->aParam[3]<=0.0 || p->aParam[4]<=0.0 || p->aParam[5]<=0.0
    ){
      واپس SQLITE_ERROR;
    }
    pCube = (Cube *)sqlite3_malloc(حجم(Cube));
    اگر( !pCube ){
      واپس SQLITE_NOMEM;
    }
    pCube->x = p->aParam[0];
    pCube->y = p->aParam[1];
    pCube->z = p->aParam[2];
    pCube->چوڑائی = p->aParam[3];
    pCube->اونچائی = p->aParam[4];
    pCube->گہرائی = p->aParam[5];

    p->pUser = (خالی *)pCube;
    p->xDelUser = cube_context_free;
  }

  أكد( nCoord==6 );
  *piRes = 0;
  اگر( aCoord[0]<=(pCube->x+pCube->چوڑائی)
   && aCoord[1]>=pCube->x
   && aCoord[2]<=(pCube->y+pCube->اونچائی)
   && aCoord[3]>=pCube->y
   && aCoord[4]<=(pCube->z+pCube->گہرائی)
   && aCoord[5]>=pCube->z
  ){
    *piRes = 1;
  }

  واپس SQLITE_OK;
}
#اگر_ختم /* SQLITE_ENABLE_RTREE */

ساکن صحیح SQLITE_TCLAPI رجسٹر_cube_geom(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
#اگر_غیر_تعریف_شدہ SQLITE_ENABLE_RTREE
  غیر_مستعمل_PARAMETER(client_ڈیٹا);
  غیر_مستعمل_PARAMETER(interp);
  غیر_مستعمل_PARAMETER(objc);
  غیر_مستعمل_PARAMETER(objv);
#ورنہ
  خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp*, ثابت حرف*, sqlite3**);
  خارجی ثابت حرف *sqlite3ErrName(صحیح);
  sqlite3 *db;
  صحیح rc;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB");
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "cube", cube_geom, (خالی *)&g_یہاں);
  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
#اگر_ختم
  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI رجسٹر_circle_geom(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
#اگر_غیر_تعریف_شدہ SQLITE_ENABLE_RTREE
  غیر_مستعمل_PARAMETER(client_ڈیٹا);
  غیر_مستعمل_PARAMETER(interp);
  غیر_مستعمل_PARAMETER(objc);
  غیر_مستعمل_PARAMETER(objv);
#ورنہ
  خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp*, ثابت حرف*, sqlite3**);
  خارجی ثابت حرف *sqlite3ErrName(صحیح);
  sqlite3 *db;
  صحیح rc;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB");
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "circle", circle_geom, 0);
  اگر( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "Qcircle",
                                      circle_query_func, 0, 0);
  }
  اگر( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "breadthfirstsearch",
                                      bfs_query_func, 0, 0);
  }
  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
#اگر_ختم
  واپس TCL_OK;
}

صحیح Sqlitetestrtree_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_cube_geom", رجسٹر_cube_geom, 0, 0);
  Tcl_CreateObjCommand(interp, "register_circle_geom",رجسٹر_circle_geom,0,0);
  واپس TCL_OK;
}
