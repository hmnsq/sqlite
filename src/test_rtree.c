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

#অন্তর্ভাব "sqlite3.h"
#অন্তর্ভাব "tclsqlite.h"

/* Solely for the UNUSED_PARAMETER() macro. */
#অন্তর্ভাব "sqliteInt.h"

#ifdef SQLITE_ENABLE_RTREE
/* 
** Type used to cache parameter information for the "circle" r-tree geometry
** callback.
*/
প্রকারলেখ কাঠামো Circle Circle;
কাঠামো Circle {
  কাঠামো Box {
    দ্বিগুণ xmin;
    দ্বিগুণ xmax;
    দ্বিগুণ ymin;
    দ্বিগুণ ymax;
  } aBox[2];
  দ্বিগুণ centerx;
  দ্বিগুণ centery;
  দ্বিগুণ radius;
  দ্বিগুণ mxArea;
  পূর্ণসংখ্যা eScoreType;
};

/*
** Destructor function for Circle objects allocated by circle_geom().
*/
স্থির শূন্য circle_del(শূন্য *p){
  sqlite3_free(p);
}

/*
** Implementation of "circle" r-tree geometry callback.
*/
স্থির পূর্ণসংখ্যা circle_geom(
  sqlite3_rtree_geometry *p,
  পূর্ণসংখ্যা nCoord, 
  sqlite3_rtree_dbl *aCoord,
  পূর্ণসংখ্যা *pRes
){
  পূর্ণসংখ্যা i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  দ্বিগুণ xmin, xmax;              /* X dimensions of box being tested */
  দ্বিগুণ ymin, ymax;              /* X dimensions of box being tested */

  xmin = aCoord[0];
  xmax = aCoord[1];
  ymin = aCoord[2];
  ymax = aCoord[3];
  pCircle = (Circle *)p->pUser;
  যদি( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    যদি( nCoord!=4 ) ফেরত SQLITE_ERROR;

    /* Test that the correct number of parameters (3) have been supplied,
    ** and that the parameters are in range (that the radius of the circle 
    ** radius is greater than zero). */
    যদি( p->nParam!=3 || p->aParam[2]<0.0 ) ফেরত SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(আকার(Circle)));
    যদি( !pCircle ) ফেরত SQLITE_NOMEM;
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
  জন্য(i=0; i<4; i++){
    দ্বিগুণ x = (i&0x01) ? xmax : xmin;
    দ্বিগুণ y = (i&0x02) ? ymax : ymin;
    দ্বিগুণ d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    যদি( d2<(pCircle->radius*pCircle->radius) ){
      *pRes = 1;
      ফেরত SQLITE_OK;
    }
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  জন্য(i=0; i<2; i++){
    যদি( xmin<=pCircle->aBox[i].xmin 
     && xmax>=pCircle->aBox[i].xmax 
     && ymin<=pCircle->aBox[i].ymin 
     && ymax>=pCircle->aBox[i].ymax 
    ){
      *pRes = 1;
      ফেরত SQLITE_OK;
    }
  }

  /* The specified bounding box does not intersect the circular region. Set
  ** the output variable to zero and return SQLITE_OK. */
  *pRes = 0;
  ফেরত SQLITE_OK;
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
স্থির পূর্ণসংখ্যা circle_query_func(sqlite3_rtree_query_info *p){
  পূর্ণসংখ্যা i;                          /* Iterator variable */
  Circle *pCircle;                /* Structure defining circular region */
  দ্বিগুণ xmin, xmax;              /* X dimensions of box being tested */
  দ্বিগুণ ymin, ymax;              /* X dimensions of box being tested */
  পূর্ণসংখ্যা nWithin = 0;                /* Number of corners inside the circle */

  xmin = p->aCoord[0];
  xmax = p->aCoord[1];
  ymin = p->aCoord[2];
  ymax = p->aCoord[3];
  pCircle = (Circle *)p->pUser;
  যদি( pCircle==0 ){
    /* If pUser is still 0, then the parameter values have not been tested
    ** for correctness or stored into a Circle structure yet. Do this now. */

    /* This geometry callback is for use with a 2-dimensional r-tree table.
    ** Return an error if the table does not have exactly 2 dimensions. */
    যদি( p->nCoord!=4 ) ফেরত SQLITE_ERROR;

    /* Test that the correct number of parameters (1 or 4) have been supplied.
    */
    যদি( p->nParam!=4 && p->nParam!=1 ) ফেরত SQLITE_ERROR;

    /* Allocate a structure to cache parameter data in. Return SQLITE_NOMEM
    ** if the allocation fails. */
    pCircle = (Circle *)(p->pUser = sqlite3_malloc(আকার(Circle)));
    যদি( !pCircle ) ফেরত SQLITE_NOMEM;
    p->xDelUser = circle_del;

    /* Record the center and radius of the circular region. One way that
    ** tested bounding boxes that intersect the circular region are detected
    ** is by testing if each corner of the bounding box lies within radius
    ** units of the center of the circle. */
    যদি( p->nParam==4 ){
      pCircle->centerx = p->aParam[0];
      pCircle->centery = p->aParam[1];
      pCircle->radius = p->aParam[2];
      pCircle->eScoreType = (পূর্ণসংখ্যা)p->aParam[3];
    }অন্যথা{
      ধ্রুবক অক্ষর *z = (ধ্রুবক অক্ষর*)sqlite3_মান_text(p->apSqlParam[0]);
      pCircle->centerx = 0.0;
      pCircle->centery = 0.0;
      pCircle->radius = 0.0;
      pCircle->eScoreType = 0;
      যখন( z && z[0] ){
        যদি( z[0]=='r' && z[1]==':' ){
          pCircle->radius = atof(&z[2]);
        }অন্যথা যদি( z[0]=='x' && z[1]==':' ){
          pCircle->centerx = atof(&z[2]);
        }অন্যথা যদি( z[0]=='y' && z[1]==':' ){
          pCircle->centery = atof(&z[2]);
        }অন্যথা যদি( z[0]=='e' && z[1]==':' ){
          pCircle->eScoreType = (পূর্ণসংখ্যা)atof(&z[2]);
        }অন্যথা যদি( z[0]==' ' ){
          z++;
          চালু;
        }
        যখন( z[0]!=0 && z[0]!=' ' ) z++;
        যখন( z[0]==' ' ) z++;
      }
    }
    যদি( pCircle->radius<0.0 ){
      sqlite3_free(pCircle);
      ফেরত SQLITE_NOMEM;
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
  জন্য(i=0; i<4; i++){
    দ্বিগুণ x = (i&0x01) ? xmax : xmin;
    দ্বিগুণ y = (i&0x02) ? ymax : ymin;
    দ্বিগুণ d2;
    
    d2  = (x-pCircle->centerx)*(x-pCircle->centerx);
    d2 += (y-pCircle->centery)*(y-pCircle->centery);
    যদি( d2<(pCircle->radius*pCircle->radius) ) nWithin++;
  }

  /* Check if the bounding box covers any other part of the circular region.
  ** See comments above for a description of how this test works. If it does
  ** cover part of the circular region, set the output variable to true
  ** and return SQLITE_OK. */
  যদি( nWithin==0 ){
    জন্য(i=0; i<2; i++){
      যদি( xmin<=pCircle->aBox[i].xmin 
       && xmax>=pCircle->aBox[i].xmax 
       && ymin<=pCircle->aBox[i].ymin 
       && ymax>=pCircle->aBox[i].ymax 
      ){
        nWithin = 1;
        বিরতি;
      }
    }
  }

  যদি( pCircle->eScoreType==1 ){
    /* Depth first search */
    p->rScore = p->iLevel;
  }অন্যথা যদি( pCircle->eScoreType==2 ){
    /* Breadth first search */
    p->rScore = 100 - p->iLevel;
  }অন্যথা যদি( pCircle->eScoreType==3 ){
    /* Depth-first search, except sort the leaf nodes by area with
    ** the largest area first */
    যদি( p->iLevel==1 ){
      p->rScore = 1.0 - (xmax-xmin)*(ymax-ymin)/pCircle->mxArea;
      যদি( p->rScore<0.01 ) p->rScore = 0.01;
    }অন্যথা{
      p->rScore = 0.0;
    }
  }অন্যথা যদি( pCircle->eScoreType==4 ){
    /* Depth-first search, except exclude odd rowids */
    p->rScore = p->iLevel;
    যদি( p->iRowid&1 ) nWithin = 0;
  }অন্যথা{
    /* Breadth-first search, except exclude odd rowids */
    p->rScore = 100 - p->iLevel;
    যদি( p->iRowid&1 ) nWithin = 0;
  }
  যদি( nWithin==0 ){
    p->eWithin = NOT_WITHIN;
  }অন্যথা যদি( nWithin>=4 ){
    p->eWithin = FULLY_WITHIN;
  }অন্যথা{
    p->eWithin = PARTLY_WITHIN;
  }
  ফেরত SQLITE_OK;
}
/*
** Implementation of "breadthfirstsearch" r-tree geometry callback using the 
** 2nd-generation interface that allows scoring.
**
**     ... WHERE id MATCH breadthfirstsearch($x0,$x1,$y0,$y1) ...
**
** It returns all entries whose bounding boxes overlap with $x0,$x1,$y0,$y1.
*/
স্থির পূর্ণসংখ্যা bfs_query_func(sqlite3_rtree_query_info *p){
  দ্বিগুণ x0,x1,y0,y1;        /* Dimensions of box being tested */
  দ্বিগুণ bx0,bx1,by0,by1;    /* Boundary of the query function */

  যদি( p->nParam!=4 ) ফেরত SQLITE_ERROR;
  x0 = p->aCoord[0];
  x1 = p->aCoord[1];
  y0 = p->aCoord[2];
  y1 = p->aCoord[3];
  bx0 = p->aParam[0];
  bx1 = p->aParam[1];
  by0 = p->aParam[2];
  by1 = p->aParam[3];
  p->rScore = 100 - p->iLevel;
  যদি( p->eParentWithin==FULLY_WITHIN ){
    p->eWithin = FULLY_WITHIN;
  }অন্যথা যদি( x0>=bx0 && x1<=bx1 && y0>=by0 && y1<=by1 ){
    p->eWithin = FULLY_WITHIN;
  }অন্যথা যদি( x1>=bx0 && x0<=bx1 && y1>=by0 && y0<=by1 ){
    p->eWithin = PARTLY_WITHIN;
  }অন্যথা{
    p->eWithin = NOT_WITHIN;
  }
  ফেরত SQLITE_OK;
}

/* END of implementation of "circle" geometry callback.
**************************************************************************
*************************************************************************/

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব "tclsqlite.h"

প্রকারলেখ কাঠামো Cube Cube;
কাঠামো Cube {
  দ্বিগুণ x;
  দ্বিগুণ y;
  দ্বিগুণ z;
  দ্বিগুণ width;
  দ্বিগুণ height;
  দ্বিগুণ depth;
};

স্থির শূন্য cube_context_free(শূন্য *p){
  sqlite3_free(p);
}

/*
** The context pointer registered along with the 'cube' callback is
** always ((void *)&gHere). This is just to facilitate testing, it is not
** actually used for anything.
*/
স্থির পূর্ণসংখ্যা gHere = 42;

/*
** Implementation of a simple r-tree geom callback to test for intersection
** of r-tree rows with a "cube" shape. Cubes are defined by six scalar
** coordinates as follows:
**
**   cube(x, y, z, width, height, depth)
**
** The width, height and depth parameters must all be greater than zero.
*/
স্থির পূর্ণসংখ্যা cube_geom(
  sqlite3_rtree_geometry *p,
  পূর্ণসংখ্যা nCoord,
  sqlite3_rtree_dbl *aCoord,
  পূর্ণসংখ্যা *piRes
){
  Cube *pCube = (Cube *)p->pUser;

  প্রতিজ্ঞা( p->pContext==(শূন্য *)&gHere );

  যদি( pCube==0 ){
    যদি( p->nParam!=6 || nCoord!=6
     || p->aParam[3]<=0.0 || p->aParam[4]<=0.0 || p->aParam[5]<=0.0
    ){
      ফেরত SQLITE_ERROR;
    }
    pCube = (Cube *)sqlite3_malloc(আকার(Cube));
    যদি( !pCube ){
      ফেরত SQLITE_NOMEM;
    }
    pCube->x = p->aParam[0];
    pCube->y = p->aParam[1];
    pCube->z = p->aParam[2];
    pCube->width = p->aParam[3];
    pCube->height = p->aParam[4];
    pCube->depth = p->aParam[5];

    p->pUser = (শূন্য *)pCube;
    p->xDelUser = cube_context_free;
  }

  প্রতিজ্ঞা( nCoord==6 );
  *piRes = 0;
  যদি( aCoord[0]<=(pCube->x+pCube->width)
   && aCoord[1]>=pCube->x
   && aCoord[2]<=(pCube->y+pCube->height)
   && aCoord[3]>=pCube->y
   && aCoord[4]<=(pCube->z+pCube->depth)
   && aCoord[5]>=pCube->z
  ){
    *piRes = 1;
  }

  ফেরত SQLITE_OK;
}
#endif /* SQLITE_ENABLE_RTREE */

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI register_cube_geom(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_ENABLE_RTREE
  UNUSED_PARAMETER(client_তথ্য);
  UNUSED_PARAMETER(interp);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
#অন্যথা
  বাহ্যিক পূর্ণসংখ্যা getDbPointer(Tcl_Interp*, ধ্রুবক অক্ষর*, sqlite3**);
  বাহ্যিক ধ্রুবক অক্ষর *sqlite3ErrName(পূর্ণসংখ্যা);
  sqlite3 *db;
  পূর্ণসংখ্যা rc;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    ফেরত TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ফেরত TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "cube", cube_geom, (শূন্য *)&gHere);
  Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3ErrName(rc), TCL_STATIC);
#endif
  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI register_circle_geom(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_ENABLE_RTREE
  UNUSED_PARAMETER(client_তথ্য);
  UNUSED_PARAMETER(interp);
  UNUSED_PARAMETER(objc);
  UNUSED_PARAMETER(objv);
#অন্যথা
  বাহ্যিক পূর্ণসংখ্যা getDbPointer(Tcl_Interp*, ধ্রুবক অক্ষর*, sqlite3**);
  বাহ্যিক ধ্রুবক অক্ষর *sqlite3ErrName(পূর্ণসংখ্যা);
  sqlite3 *db;
  পূর্ণসংখ্যা rc;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    ফেরত TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ফেরত TCL_ERROR;
  rc = sqlite3_rtree_geometry_callback(db, "circle", circle_geom, 0);
  যদি( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "Qcircle",
                                      circle_query_func, 0, 0);
  }
  যদি( rc==SQLITE_OK ){
    rc = sqlite3_rtree_query_callback(db, "breadthfirstsearch",
                                      bfs_query_func, 0, 0);
  }
  Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3ErrName(rc), TCL_STATIC);
#endif
  ফেরত TCL_OK;
}

পূর্ণসংখ্যা Sqlitetestrtree_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_cube_geom", register_cube_geom, 0, 0);
  Tcl_CreateObjCommand(interp, "register_circle_geom",register_circle_geom,0,0);
  ফেরত TCL_OK;
}
