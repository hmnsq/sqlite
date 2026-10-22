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

#समावेश "sqlite3.h"
#समावेश "tclsqlite.h"

/* Solely for the UNUSED_PARAMETER() macro. */
#समावेश "sqliteInt.h"

#अगर_घोषित SQLITE_ENABLE_RTREE
/* 
** Type used to cache parameter information for the "circle" r-tree geometry
** callback.
*/
प्रकार काष्ठा Circle Circle;
काष्ठा Circle {
  काष्ठा Box {
    द्विगुन xmin;
    द्विगुन xmax;
    द्विगुन ymin;
    द्विगुन ymax;
  } aBox[2];
  द्विगुन centerx;
  द्विगुन centery;
  द्विगुन radius;
  द्विगुन mxArea;
  पूर्णांक eScoreType;
};

/*
** Destructor function for Circle objects allocated by circle_geom().
*/
अटल व्योम circle_del(व्योम *p){
  sqlite3_free(p);
}

/*
** Implementation of "circle" r-tree geometry callback.
*/
अटल पूर्णांक circle_geom(
  sqlite3_rtree_geometry *p,
  पूर्णांक nCoord, 
  sqlite3_rtree_dbl *aCoord,
  पूर्णांक *pRes
){
  पूर्णांक i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  द्विगुन xmin, xmax;              /* X dimensions of box being tested */
  द्विगुन ymin, ymax;              /* X dimensions of box being tested */

  xmin = aCoord[0];
  xmax = aCoord[1];
  ymin = aCoord[2];
  ymax = aCoord[3];
  pCircle = (Circle *)p->pUser;
  अगर( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    अगर( nCoord!=4 ) वापस SQLITE_ERROR;

    /* Test that the correct number of parameters (3) have been supplied,
    ** and that the parameters are in range (that the radius of the circle 
    ** radius is greater than zero). */
    अगर( p->nParam!=3 || p->aParam[2]<0.0 ) वापस SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(माप(Circle)));
    अगर( !pCircle ) वापस SQLITE_NOMEM;
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
  क्रम(i=0; i<4; i++){
    द्विगुन x = (i&0x01) ? xmax : xmin;
    द्विगुन y = (i&0x02) ? ymax : ymin;
    द्विगुन d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    अगर( d2<(pCircle->radius*pCircle->radius) ){
      *pRes = 1;
      वापस SQLITE_OK;
    }
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  क्रम(i=0; i<2; i++){
    अगर( xmin<=pCircle->aBox[i].xmin 
     && xmax>=pCircle->aBox[i].xmax 
     && ymin<=pCircle->aBox[i].ymin 
     && ymax>=pCircle->aBox[i].ymax 
    ){
      *pRes = 1;
      वापस SQLITE_OK;
    }
  }

  /* The specified bounding box does not intersect the circular region. Set
  ** the output variable to zero and return SQLITE_OK. */
  *pRes = 0;
  वापस SQLITE_OK;
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
अटल पूर्णांक circle_query_func(sqlite3_rtree_query_info *p){
  पूर्णांक i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  द्विगुन xmin, xmax;              /* X dimensions of box being tested */
  द्विगुन ymin, ymax;              /* X dimensions of box being tested */
  पूर्णांक nWithin = 0;                /* Number of corners inside the circle */

  xmin = p->aCoord[0];
  xmax = p->aCoord[1];
  ymin = p->aCoord[2];
  ymax = p->aCoord[3];
  pCircle = (Circle *)p->pUser;
  अगर( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    अगर( p->nCoord!=4 ) वापस SQLITE_ERROR;

    /* Test that the correct number of parameters (1 or 4) have been supplied.
    */
    अगर( p->nParam!=4 && p->nParam!=1 ) वापस SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(माप(Circle)));
    अगर( !pCircle ) वापस SQLITE_NOMEM;
    p->xDelUser = circle_del;

    /* Record the center and radius of the circular region. One way that
    ** tested bounding boxes that intersect the circular region are detected
    ** is by testing if each corner of the bounding box lies within radius
    ** units of the center of the circle. */
    अगर( p->nParam==4 ){
      pCircle->centerx = p->aParam[0];
      pCircle->centery = p->aParam[1];
      pCircle->radius = p->aParam[2];
      pCircle->eScoreType = (पूर्णांक)p->aParam[3];
    }अन्यथा{
      स्थिर अक्षर *z = (स्थिर अक्षर*)sqlite3_मूल्य_text(p->apSqlParam[0]);
      pCircle->centerx = 0.0;
      pCircle->centery = 0.0;
      pCircle->radius = 0.0;
      pCircle->eScoreType = 0;
      जबतक( z && z[0] ){
        अगर( z[0]=='r' && z[1]==':' ){
          pCircle->radius = म_से_भ(&z[2]);
        }अन्यथा अगर( z[0]=='x' && z[1]==':' ){
          pCircle->centerx = म_से_भ(&z[2]);
        }अन्यथा अगर( z[0]=='y' && z[1]==':' ){
          pCircle->centery = म_से_भ(&z[2]);
        }अन्यथा अगर( z[0]=='e' && z[1]==':' ){
          pCircle->eScoreType = (पूर्णांक)म_से_भ(&z[2]);
        }अन्यथा अगर( z[0]==' ' ){
          z++;
          जारी;
        }
        जबतक( z[0]!=0 && z[0]!=' ' ) z++;
        जबतक( z[0]==' ' ) z++;
      }
    }
    अगर( pCircle->radius<0.0 ){
      sqlite3_free(pCircle);
      वापस SQLITE_NOMEM;
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
  क्रम(i=0; i<4; i++){
    द्विगुन x = (i&0x01) ? xmax : xmin;
    द्विगुन y = (i&0x02) ? ymax : ymin;
    द्विगुन d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    अगर( d2<(pCircle->radius*pCircle->radius) ) nWithin++;
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  अगर( nWithin==0 ){
    क्रम(i=0; i<2; i++){
      अगर( xmin<=pCircle->aBox[i].xmin 
       && xmax>=pCircle->aBox[i].xmax 
       && ymin<=pCircle->aBox[i].ymin 
       && ymax>=pCircle->aBox[i].ymax 
      ){
        nWithin = 1;
        अवरोध;
      }
    }
  }

  अगर( pCircle->eScoreType==1 ){
    /* Depth first search */
    p->rScore = p->iLevel;
  }अन्यथा अगर( pCircle->eScoreType==2 ){
    /* Breadth first search */
    p->rScore = 100 - p->iLevel;
  }अन्यथा अगर( pCircle->eScoreType==3 ){
    /* Depth-first search, except sort the leaf nodes by area with
    ** the largest area first */
    अगर( p->iLevel==1 ){
      p->rScore = 1.0 - (xmax-xmin)*(ymax-ymin)/pCircle->mxArea;
      अगर( p->rScore<0.01 ) p->rScore = 0.01;
    }अन्यथा{
      p->rScore = 0.0;
    }
  }अन्यथा अगर( pCircle->eScoreType==4 ){
    /* Depth-first search, except exclude odd rowids */
    p->rScore = p->iLevel;
    अगर( p->iRowid&1 ) nWithin = 0;
  }अन्यथा{
    /* Breadth-first search, except exclude odd rowids */
    p->rScore = 100 - p->iLevel;
    अगर( p->iRowid&1 ) nWithin = 0;
  }
  अगर( nWithin==0 ){
    p->eWithin = NOT_WITHIN;
  }अन्यथा अगर( nWithin>=4 ){
    p->eWithin = FULLY_WITHIN;
  }अन्यथा{
    p->eWithin = PARTLY_WITHIN;
  }
  वापस SQLITE_OK;
}
/*
** Implementation of "breadthfirstsearch" r-tree geometry callback using the 
** 2nd-generation interface that allows scoring.
**
**     ... WHERE id MATCH breadthfirstsearch($x0,$x1,$y0,$y1) ...
**
** It returns all entries whose bounding boxes overlap with $x0,$x1,$y0,$y1.
*/
अटल पूर्णांक bfs_query_func(sqlite3_rtree_query_info *p){
  द्विगुन x0,x1,y0,y1;        /* Dimensions of box being tested */
  द्विगुन bx0,bx1,by0,by1;    /* Boundary of the query function */

  अगर( p->nParam!=4 ) वापस SQLITE_ERROR;
  x0 = p->aCoord[0];
  x1 = p->aCoord[1];
  y0 = p->aCoord[2];
  y1 = p->aCoord[3];
  bx0 = p->aParam[0];
  bx1 = p->aParam[1];
  by0 = p->aParam[2];
  by1 = p->aParam[3];
  p->rScore = 100 - p->iLevel;
  अगर( p->eParentWithin==FULLY_WITHIN ){
    p->eWithin = FULLY_WITHIN;
  }अन्यथा अगर( x0>=bx0 && x1<=bx1 && y0>=by0 && y1<=by1 ){
    p->eWithin = FULLY_WITHIN;
  }अन्यथा अगर( x1>=bx0 && x0<=bx1 && y1>=by0 && y0<=by1 ){
    p->eWithin = PARTLY_WITHIN;
  }अन्यथा{
    p->eWithin = NOT_WITHIN;
  }
  वापस SQLITE_OK;
}

/* END of implementation of "circle" geometry callback.
**************************************************************************
*************************************************************************/

#समावेश <assert.h>
#समावेश "tclsqlite.h"

प्रकार काष्ठा Cube Cube;
काष्ठा Cube {
  द्विगुन x;
  द्विगुन y;
  द्विगुन z;
  द्विगुन width;
  द्विगुन height;
  द्विगुन depth;
};

अटल व्योम cube_context_free(व्योम *p){
  sqlite3_free(p);
}

/*
** The context pointer registered along with the 'cube' callback is
** always ((void *)&gHere). This is just to facilitate testing, it is not
** actually used for anything.
*/
अटल पूर्णांक gHere = 42;

/*
** Implementation of a simple r-tree geom callback to test for intersection
** of r-tree rows with a "cube" shape. Cubes are defined by six scalar
** coordinates as follows:
**
**   cube(x, y, z, width, height, depth)
**
** The width, height and depth parameters must all be greater than zero.
*/
अटल पूर्णांक cube_geom(
  sqlite3_rtree_geometry *p,
  पूर्णांक nCoord,
  sqlite3_rtree_dbl *aCoord,
  पूर्णांक *piRes
){
  Cube *pCube = (Cube *)p->pUser;

  निश्चित( p->pContext==(व्योम *)&gHere );

  अगर( pCube==0 ){
    अगर( p->nParam!=6 || nCoord!=6
     || p->aParam[3]<=0.0 || p->aParam[4]<=0.0 || p->aParam[5]<=0.0
    ){
      वापस SQLITE_ERROR;
    }
    pCube = (Cube *)sqlite3_malloc(माप(Cube));
    अगर( !pCube ){
      वापस SQLITE_NOMEM;
    }
    pCube->x = p->aParam[0];
    pCube->y = p->aParam[1];
    pCube->z = p->aParam[2];
    pCube->width = p->aParam[3];
    pCube->height = p->aParam[4];
    pCube->depth = p->aParam[5];

    p->pUser = (व्योम *)pCube;
    p->xDelUser = cube_context_free;
  }

  निश्चित( nCoord==6 );
  *piRes = 0;
  अगर( aCoord[0]<=(pCube->x+pCube->width)
   && aCoord[1]>=pCube->x
   && aCoord[2]<=(pCube->y+pCube->height)
   && aCoord[3]>=pCube->y
   && aCoord[4]<=(pCube->z+pCube->depth)
   && aCoord[5]>=pCube->z
  ){
    *piRes = 1;
  }

  वापस SQLITE_OK;
}
#पूर्ण_अगर /* SQLITE_ENABLE_RTREE */

अटल पूर्णांक SQLITE_TCLAPI register_cube_geom(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
#अगर_अघोषित SQLITE_ENABLE_RTREE
  UNUSED_PARAMETER(client_तथ्य);
  UNUSED_PARAMETER(interp);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
#अन्यथा
  बाह्य पूर्णांक getDbPointer(Tcl_Interp*, स्थिर अक्षर*, sqlite3**);
  बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);
  sqlite3 *db;
  पूर्णांक rc;

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "cube", cube_geom, (व्योम *)&gHere);
  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_STATIC);
#पूर्ण_अगर
  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI register_circle_geom(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
#अगर_अघोषित SQLITE_ENABLE_RTREE
  UNUSED_PARAMETER(client_तथ्य);
  UNUSED_PARAMETER(interp);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
#अन्यथा
  बाह्य पूर्णांक getDbPointer(Tcl_Interp*, स्थिर अक्षर*, sqlite3**);
  बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);
  sqlite3 *db;
  पूर्णांक rc;

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "circle", circle_geom, 0);
  अगर( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "Qcircle",
                                      circle_query_func, 0, 0);
  }
  अगर( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "breadthfirstsearch",
                                      bfs_query_func, 0, 0);
  }
  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_STATIC);
#पूर्ण_अगर
  वापस TCL_OK;
}

पूर्णांक Sqlitetestrtree_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_cube_geom", register_cube_geom, 0, 0);
  Tcl_CreateObjCommand(interp, "register_circle_geom",register_circle_geom,0,0);
  वापस TCL_OK;
}
