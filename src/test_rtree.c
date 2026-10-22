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

#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव "tclsqlite.h"

/* Solely for the UNUSED_PARAMETER() macro. */
#अन्तर्भाव "sqliteInt.h"

#ifdef SQLITE_ENABLE_RTREE
/* 
** Type used to cache parameter information for the "circle" r-tree geometry
** callback.
*/
प्रकारनिर्देश संरचना Circle Circle;
संरचना Circle {
  संरचना Box {
    द्विगुण xmin;
    द्विगुण xmax;
    द्विगुण ymin;
    द्विगुण ymax;
  } aBox[2];
  द्विगुण centerx;
  द्विगुण centery;
  द्विगुण radius;
  द्विगुण mxArea;
  पूर्णाङ्क eScoreType;
};

/*
** Destructor function for Circle objects allocated by circle_geom().
*/
स्थिर शून्य circle_del(शून्य *p){
  sqlite3_free(p);
}

/*
** Implementation of "circle" r-tree geometry callback.
*/
स्थिर पूर्णाङ्क circle_geom(
  sqlite3_rtree_geometry *p,
  पूर्णाङ्क nCoord, 
  sqlite3_rtree_dbl *aCoord,
  पूर्णाङ्क *pRes
){
  पूर्णाङ्क i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  द्विगुण xmin, xmax;              /* X dimensions of box being tested */
  द्विगुण ymin, ymax;              /* X dimensions of box being tested */

  xmin = aCoord[0];
  xmax = aCoord[1];
  ymin = aCoord[2];
  ymax = aCoord[3];
  pCircle = (Circle *)p->pUser;
  यदि( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    यदि( nCoord!=4 ) प्रत्यागम SQLITE_ERROR;

    /* Test that the correct number of parameters (3) have been supplied,
    ** and that the parameters are in range (that the radius of the circle 
    ** radius is greater than zero). */
    यदि( p->nParam!=3 || p->aParam[2]<0.0 ) प्रत्यागम SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(परिमाण(Circle)));
    यदि( !pCircle ) प्रत्यागम SQLITE_NOMEM;
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
  प्रति(i=0; i<4; i++){
    द्विगुण x = (i&0x01) ? xmax : xmin;
    द्विगुण y = (i&0x02) ? ymax : ymin;
    द्विगुण d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    यदि( d2<(pCircle->radius*pCircle->radius) ){
      *pRes = 1;
      प्रत्यागम SQLITE_OK;
    }
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  प्रति(i=0; i<2; i++){
    यदि( xmin<=pCircle->aBox[i].xmin 
     && xmax>=pCircle->aBox[i].xmax 
     && ymin<=pCircle->aBox[i].ymin 
     && ymax>=pCircle->aBox[i].ymax 
    ){
      *pRes = 1;
      प्रत्यागम SQLITE_OK;
    }
  }

  /* The specified bounding box does not intersect the circular region. Set
  ** the output variable to zero and return SQLITE_OK. */
  *pRes = 0;
  प्रत्यागम SQLITE_OK;
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
स्थिर पूर्णाङ्क circle_query_func(sqlite3_rtree_query_info *p){
  पूर्णाङ्क i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  द्विगुण xmin, xmax;              /* X dimensions of box being tested */
  द्विगुण ymin, ymax;              /* X dimensions of box being tested */
  पूर्णाङ्क nWithin = 0;                /* Number of corners inside the circle */

  xmin = p->aCoord[0];
  xmax = p->aCoord[1];
  ymin = p->aCoord[2];
  ymax = p->aCoord[3];
  pCircle = (Circle *)p->pUser;
  यदि( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    यदि( p->nCoord!=4 ) प्रत्यागम SQLITE_ERROR;

    /* Test that the correct number of parameters (1 or 4) have been supplied.
    */
    यदि( p->nParam!=4 && p->nParam!=1 ) प्रत्यागम SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(परिमाण(Circle)));
    यदि( !pCircle ) प्रत्यागम SQLITE_NOMEM;
    p->xDelUser = circle_del;

    /* Record the center and radius of the circular region. One way that
    ** tested bounding boxes that intersect the circular region are detected
    ** is by testing if each corner of the bounding box lies within radius
    ** units of the center of the circle. */
    यदि( p->nParam==4 ){
      pCircle->centerx = p->aParam[0];
      pCircle->centery = p->aParam[1];
      pCircle->radius = p->aParam[2];
      pCircle->eScoreType = (पूर्णाङ्क)p->aParam[3];
    }अन्यथा{
      ध्रुव वर्ण *z = (ध्रुव वर्ण*)sqlite3_मूल्य_text(p->apSqlParam[0]);
      pCircle->centerx = 0.0;
      pCircle->centery = 0.0;
      pCircle->radius = 0.0;
      pCircle->eScoreType = 0;
      यावत्( z && z[0] ){
        यदि( z[0]=='r' && z[1]==':' ){
          pCircle->radius = atof(&z[2]);
        }अन्यथा यदि( z[0]=='x' && z[1]==':' ){
          pCircle->centerx = atof(&z[2]);
        }अन्यथा यदि( z[0]=='y' && z[1]==':' ){
          pCircle->centery = atof(&z[2]);
        }अन्यथा यदि( z[0]=='e' && z[1]==':' ){
          pCircle->eScoreType = (पूर्णाङ्क)atof(&z[2]);
        }अन्यथा यदि( z[0]==' ' ){
          z++;
          अनुवर्त;
        }
        यावत्( z[0]!=0 && z[0]!=' ' ) z++;
        यावत्( z[0]==' ' ) z++;
      }
    }
    यदि( pCircle->radius<0.0 ){
      sqlite3_free(pCircle);
      प्रत्यागम SQLITE_NOMEM;
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
  प्रति(i=0; i<4; i++){
    द्विगुण x = (i&0x01) ? xmax : xmin;
    द्विगुण y = (i&0x02) ? ymax : ymin;
    द्विगुण d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    यदि( d2<(pCircle->radius*pCircle->radius) ) nWithin++;
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  यदि( nWithin==0 ){
    प्रति(i=0; i<2; i++){
      यदि( xmin<=pCircle->aBox[i].xmin 
       && xmax>=pCircle->aBox[i].xmax 
       && ymin<=pCircle->aBox[i].ymin 
       && ymax>=pCircle->aBox[i].ymax 
      ){
        nWithin = 1;
        भङ्ग;
      }
    }
  }

  यदि( pCircle->eScoreType==1 ){
    /* Depth first search */
    p->rScore = p->iLevel;
  }अन्यथा यदि( pCircle->eScoreType==2 ){
    /* Breadth first search */
    p->rScore = 100 - p->iLevel;
  }अन्यथा यदि( pCircle->eScoreType==3 ){
    /* Depth-first search, except sort the leaf nodes by area with
    ** the largest area first */
    यदि( p->iLevel==1 ){
      p->rScore = 1.0 - (xmax-xmin)*(ymax-ymin)/pCircle->mxArea;
      यदि( p->rScore<0.01 ) p->rScore = 0.01;
    }अन्यथा{
      p->rScore = 0.0;
    }
  }अन्यथा यदि( pCircle->eScoreType==4 ){
    /* Depth-first search, except exclude odd rowids */
    p->rScore = p->iLevel;
    यदि( p->iRowid&1 ) nWithin = 0;
  }अन्यथा{
    /* Breadth-first search, except exclude odd rowids */
    p->rScore = 100 - p->iLevel;
    यदि( p->iRowid&1 ) nWithin = 0;
  }
  यदि( nWithin==0 ){
    p->eWithin = NOT_WITHIN;
  }अन्यथा यदि( nWithin>=4 ){
    p->eWithin = FULLY_WITHIN;
  }अन्यथा{
    p->eWithin = PARTLY_WITHIN;
  }
  प्रत्यागम SQLITE_OK;
}
/*
** Implementation of "breadthfirstsearch" r-tree geometry callback using the 
** 2nd-generation interface that allows scoring.
**
**     ... WHERE id MATCH breadthfirstsearch($x0,$x1,$y0,$y1) ...
**
** It returns all entries whose bounding boxes overlap with $x0,$x1,$y0,$y1.
*/
स्थिर पूर्णाङ्क bfs_query_func(sqlite3_rtree_query_info *p){
  द्विगुण x0,x1,y0,y1;        /* Dimensions of box being tested */
  द्विगुण bx0,bx1,by0,by1;    /* Boundary of the query function */

  यदि( p->nParam!=4 ) प्रत्यागम SQLITE_ERROR;
  x0 = p->aCoord[0];
  x1 = p->aCoord[1];
  y0 = p->aCoord[2];
  y1 = p->aCoord[3];
  bx0 = p->aParam[0];
  bx1 = p->aParam[1];
  by0 = p->aParam[2];
  by1 = p->aParam[3];
  p->rScore = 100 - p->iLevel;
  यदि( p->eParentWithin==FULLY_WITHIN ){
    p->eWithin = FULLY_WITHIN;
  }अन्यथा यदि( x0>=bx0 && x1<=bx1 && y0>=by0 && y1<=by1 ){
    p->eWithin = FULLY_WITHIN;
  }अन्यथा यदि( x1>=bx0 && x0<=bx1 && y1>=by0 && y0<=by1 ){
    p->eWithin = PARTLY_WITHIN;
  }अन्यथा{
    p->eWithin = NOT_WITHIN;
  }
  प्रत्यागम SQLITE_OK;
}

/* END of implementation of "circle" geometry callback.
**************************************************************************
*************************************************************************/

#अन्तर्भाव <assert.h>
#अन्तर्भाव "tclsqlite.h"

प्रकारनिर्देश संरचना Cube Cube;
संरचना Cube {
  द्विगुण x;
  द्विगुण y;
  द्विगुण z;
  द्विगुण width;
  द्विगुण height;
  द्विगुण depth;
};

स्थिर शून्य cube_context_free(शून्य *p){
  sqlite3_free(p);
}

/*
** The context pointer registered along with the 'cube' callback is
** always ((void *)&gHere). This is just to facilitate testing, it is not
** actually used for anything.
*/
स्थिर पूर्णाङ्क gHere = 42;

/*
** Implementation of a simple r-tree geom callback to test for intersection
** of r-tree rows with a "cube" shape. Cubes are defined by six scalar
** coordinates as follows:
**
**   cube(x, y, z, width, height, depth)
**
** The width, height and depth parameters must all be greater than zero.
*/
स्थिर पूर्णाङ्क cube_geom(
  sqlite3_rtree_geometry *p,
  पूर्णाङ्क nCoord,
  sqlite3_rtree_dbl *aCoord,
  पूर्णाङ्क *piRes
){
  Cube *pCube = (Cube *)p->pUser;

  प्रतिज्ञा( p->pContext==(शून्य *)&gHere );

  यदि( pCube==0 ){
    यदि( p->nParam!=6 || nCoord!=6
     || p->aParam[3]<=0.0 || p->aParam[4]<=0.0 || p->aParam[5]<=0.0
    ){
      प्रत्यागम SQLITE_ERROR;
    }
    pCube = (Cube *)sqlite3_malloc(परिमाण(Cube));
    यदि( !pCube ){
      प्रत्यागम SQLITE_NOMEM;
    }
    pCube->x = p->aParam[0];
    pCube->y = p->aParam[1];
    pCube->z = p->aParam[2];
    pCube->width = p->aParam[3];
    pCube->height = p->aParam[4];
    pCube->depth = p->aParam[5];

    p->pUser = (शून्य *)pCube;
    p->xDelUser = cube_context_free;
  }

  प्रतिज्ञा( nCoord==6 );
  *piRes = 0;
  यदि( aCoord[0]<=(pCube->x+pCube->width)
   && aCoord[1]>=pCube->x
   && aCoord[2]<=(pCube->y+pCube->height)
   && aCoord[3]>=pCube->y
   && aCoord[4]<=(pCube->z+pCube->depth)
   && aCoord[5]>=pCube->z
  ){
    *piRes = 1;
  }

  प्रत्यागम SQLITE_OK;
}
#endif /* SQLITE_ENABLE_RTREE */

स्थिर पूर्णाङ्क SQLITE_TCLAPI register_cube_geom(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_ENABLE_RTREE
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(interp);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
#अन्यथा
  बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp*, ध्रुव वर्ण*, sqlite3**);
  बाह्य ध्रुव वर्ण *sqlite3ErrName(पूर्णाङ्क);
  sqlite3 *db;
  पूर्णाङ्क rc;

  यदि( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    प्रत्यागम TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "cube", cube_geom, (शून्य *)&gHere);
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);
#endif
  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI register_circle_geom(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_ENABLE_RTREE
  UNUSED_PARAMETER(clientData);
  UNUSED_PARAMETER(interp);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
#अन्यथा
  बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp*, ध्रुव वर्ण*, sqlite3**);
  बाह्य ध्रुव वर्ण *sqlite3ErrName(पूर्णाङ्क);
  sqlite3 *db;
  पूर्णाङ्क rc;

  यदि( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    प्रत्यागम TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "circle", circle_geom, 0);
  यदि( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "Qcircle",
                                      circle_query_func, 0, 0);
  }
  यदि( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "breadthfirstsearch",
                                      bfs_query_func, 0, 0);
  }
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);
#endif
  प्रत्यागम TCL_OK;
}

पूर्णाङ्क Sqlitetestrtree_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_cube_geom", register_cube_geom, 0, 0);
  Tcl_CreateObjCommand(interp, "register_circle_geom",register_circle_geom,0,0);
  प्रत्यागम TCL_OK;
}
