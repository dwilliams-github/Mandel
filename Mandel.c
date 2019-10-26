/*
 | Plot Mandelbrot set
 |
 | David C. Williams    5/14/90
*/

#include <MacHeaders.h>

#include <math.h>

#define		MAIN_WIND	128

#define		APPLE_MENU	128
#define		APPLE_ABOUT	1

#define		FILE_MENU	129
#define		FILE_QUIT	1

#define		EDIT_MENU	130

#define		OPTION_MENU		131
#define	    OPTION_GO		1
#define		OPTION_SCALE	2

MenuHandle 		menus[4];

CursHandle		watchCurs;

WindowRecord	wStorage;				/* Storage for window record */

typedef struct {
 	WindowPtr	window;
 	int			i;
 	double		top, 
 				left, 
 				size;
 	double		newTop,
 				newLeft,
 				newSize;
} MandViewType;

PicHandle		saveMand;
MandViewType	MandView;

main()
{	
	/* Initialize all the system stuff */

	InitGraf( &thePort );
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( 0L );
	
	/* Get handle to watch cursor */
	
	watchCurs = GetCursor( watchCursor );
		
	/* Make our menus */

	MakeMenus();						
	
	/* Make our window */

	MandView.window = GetNewWindow( MAIN_WIND, &wStorage, (void *)-1L );
	SelectWindow( MandView.window );

	/* Wait for event */

	EventLoop( &MandView ); 
	
	/* Finish up */
		
	CloseWindow( MandView.window );
	
	ExitToShell();
}


MakeMenus()
{
	int				menu;
		
	/* Make Apple menu, add standard DA menu items */
	
	menus[0] = GetMenu( APPLE_MENU );
	AddResMenu( menus[0], 'DRVR' );
	
	/* Add our file menu */
	
	menus[1] = GetMenu( FILE_MENU );
	
	/* Add the default edit menu */
	
	menus[2] = GetMenu( EDIT_MENU );
	
	/* Add the "Try Me" menu */
	
	menus[3] = GetMenu( OPTION_MENU );
	
	/* Insert the menus in order onto the menu list */
	
	for (menu=0;menu<4;menu++) InsertMenu( menus[menu], 0 );
	
	/* Draw the menus */
	
	DrawMenuBar();
	
	return;
}


EventLoop( MandView )
	MandViewType	*MandView;
{
	EventRecord		doneEvent;
	WindowPtr 		whichWindow;
	int 			whereCode, size;
	Rect     		moveBound,
					newChar 	= {35,40,60,70};
	GrafPtr			wPort;
	Point			localWhere;
						
	FlushEvents( everyEvent, 0 );
	
	/* Calculate the move bounds for the window */
	
	GetWMgrPort( &wPort );
	moveBound.left = wPort->portRect.left;
	moveBound.top = wPort->portRect.top + 16;
	moveBound.right = wPort->portRect.right - 4;
	moveBound.bottom = wPort->portRect.bottom - 4; 
	
	/* Initialize mandel picture */
	
	MandView->i = 1;
	MandView->top = -1.4;
	MandView->left = -2.2;
	MandView->size = 2.8;

	/* Wait for "goaway" event */
	
	for(;;) {
	
		/* Change the mouse to a pointer */
		
		InitCursor();
		
		/* Wait for next update or mouse down event */
		
		while( !GetNextEvent( everyEvent, &doneEvent )) SystemTask();
		
		switch(doneEvent.what) {
			case mouseDown:	
			
				/* Mouse was pressed - find out where */
				
				whereCode = FindWindow( doneEvent.where, &whichWindow );
				switch (whereCode) {
					case inGoAway:
						TrackGoAway( whichWindow, doneEvent.where );
						return;
				 		break;
				 	case inSysWindow:
				 		SystemClick( &doneEvent, whichWindow );
				 		break;
				 	case inDrag:
				 		DragWindow( whichWindow, doneEvent.where, &moveBound );
				 		break;
				 	case inContent:
				 	
				 		localWhere.h = doneEvent.where.h;
				 		localWhere.v = doneEvent.where.v;
				 		GlobalToLocal( &localWhere );
				 		
				 		SetPort( MandView->window );
				 		NewSquare( &localWhere, &size );
				 		
				 		MandView->newTop = 
				 			(MandView->size/800.0)*((float) localWhere.v) +
				 				MandView->top;
				 		MandView->newLeft =
				 			(MandView->size/800.0)*((float) localWhere.h) +
				 				MandView->left;
				 		MandView->newSize = MandView->size * ((float)size)/800.0;

				 		break;
				 	case inMenuBar:
				 		if (!DoMenu( doneEvent.where, MandView )) return;
				 		break;
				 }
		 		break;
		 	case updateEvt:
				BeginUpdate( MandView->window );
				EndUpdate( MandView->window );	
				break;
			default:;
	 	}
	}
}

int	DoMenu( where, MandView )
	Point			where;
	MandViewType	*MandView;
{
	union {
		short		byWord[2];
		long		byLong;
	} menuId;
	
	unsigned char			appleName[255];
	
	/* Get menu selection */
	
	menuId.byLong = MenuSelect( where );
	
	if (menuId.byWord[1]!=0) {
	
	 	/* Valid menu selection */
	 	
	 	switch(menuId.byWord[0]) {
	 	
	 		case APPLE_MENU:					/* In Apple Menu: display 'about' info */
	 			switch(menuId.byWord[1]) {
	 				case APPLE_ABOUT:
		 			/*	ShowAbout(); */
		 				break;
		 			default:
		 				GetItem( menus[0], menuId.byWord[1], appleName );
		 				OpenDeskAcc( appleName );
		 		}
		 		break;
			case FILE_MENU:					/* In File Menu */
	 			switch(menuId.byWord[1]) {
	 				case FILE_QUIT:
	 					HiliteMenu(0);
	 					return(FALSE);
	 					break;
	 			}
	 			break;
	 		case OPTION_MENU:
	 			switch(menuId.byWord[1]) {
	 				case OPTION_GO:
	 				
	 					/* Make picture */
 	
 	 					MakeMandel( MandView );
	 					break;
	 				case OPTION_SCALE:
	 				
	 					/* Make scaled view */
	 					
	 					MandView->i = 1;
	 					MandView->top = MandView->newTop;
	 					MandView->left = MandView->newLeft;
	 					MandView->size = MandView->newSize;
	 					
						EraseRect( &MandView->window->portRect ); 
	
	 					MakeMandel( MandView );
	 					
	 					break;
	 			}
	 	}
	}
	
	HiliteMenu(0);
	
	return(TRUE);
}



MakeMandel( MandView )
	MandViewType	*MandView;
{
	double	z[2];
	int		j, iterations;
	EventRecord		doneEvent;
	Rect		test;
	Point		localEvent;
	int			drawState;

	/* Make the mouse cursor a watch */
	
	SetCursor( *watchCurs );
	
	/* Set port to mandel window */

	SetPort( MandView->window );

	/* Decide how many iterations needed at this scale */
	
	iterations = 100/MandView->size;
	
	for (; MandView->i<801; MandView->i++ ) {
		if (GetNextEvent( everyEvent, &doneEvent )) break;
		SystemTask();
		drawState = 0;	  	
		for (j=0; j<801; j++ ) {
	   
	     	/* Try this pixel */
	      
	      	z[0] = (MandView->size/800.0)*((float) MandView->i) + MandView->left;
	      	z[1] = (MandView->size/800.0)*((float) j) + MandView->top;
	      
	      	if (MandConverge(z,iterations)) {
	      		if (!drawState) {
	      			MoveTo(MandView->i,j);
	      			drawState = 1;
	      		}
	      	}
	      	else if (drawState) {
	      	 	LineTo(MandView->i,j-1);
	      	 	drawState = 0;
	      	}
	    }
	    if (drawState) LineTo(MandView->i,j-1);
	}
	
	return;
}


int MandConverge(z, n)
	double	*z;
	int		n;
{
	double	z1[2], ztemp;
	int		i;

	z1[0] = 0.0;
	z1[1] = 0.0;
	for(i=0;i<n;i++) {
		ztemp = z1[0]*z1[0] - z1[1]*z1[1] + z[0];
		z1[1] = 2.0*z1[0]*z1[1] + z[1];
		z1[0] = ztemp;
		
		if ( (z1[0]< -2.0) || (z1[0] > 2.0) ||
		     (z1[1]< -2.0) || (z1[1] > 2.0)   ) return(0);
	}
	
	return(1);
}


NewSquare( where, size )
	Point		*where;
	int			*size;
/*
 | Draw selection square in window "window" starting at "where".
 | "newRect" is the selected square.
*/
{
	Point	deltaPoint;
	int		size2;
	Rect	newRect;
	
	EventRecord	leftEvent;
	
	PenState	penState;
	
	/* Get pen state (to be returned at end) */
	
	GetPenState( &penState );
	
	/* Set pen mode to Xor and pattern to gray */
	
	PenMode( patXor );
	PenPat( gray );
	
	/* Initialize newRect->top to -1 to indicate first interation */
	
	newRect.top = -20000;
			
	for(;;) {
		/* Clear last rectangle */
		
		if (newRect.top != -20000 ) {
			FrameRect( &newRect );
			if (!StillDown()) break;
		}
			
		/* where is the mouse ? */

		GetMouse( &deltaPoint );					
		deltaPoint.v -= where->v;
		deltaPoint.h -= where->h;
			
		/* Get smallest vert,horz dist */

		*size  = deltaPoint.h < 0 ? -deltaPoint.h : deltaPoint.h;
		size2 = deltaPoint.v < 0 ? -deltaPoint.v : deltaPoint.v;
		if (*size<size2) *size = size2;

		/* Form a rectangle */

		if (deltaPoint.h>0) {
		 	newRect.left = where->h;
		 	newRect.right = newRect.left + *size;
		}
		else {
		 	newRect.right = where->h;
		 	newRect.left = newRect.right - *size;
		}
		 
		if (deltaPoint.v>0) {
		 	newRect.top = where->v;
		 	newRect.bottom = newRect.top + *size;
		}
		else {
		 	newRect.bottom = where->v;
		 	newRect.top = newRect.bottom - *size;
		}
		 
		 /* Draw the rectangle */
		
		FrameRect( &newRect );
	}
	
	/* Get the leftover mouse up event */
	
	GetNextEvent( mUpMask, &leftEvent );
		
	SetPenState( &penState );
	
	return;
}


