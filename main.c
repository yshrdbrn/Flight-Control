#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <gtk/gtk.h>


GtkWidget *window, *darea;
int windowWidth = 960, windowHeight = 624;
int playButtonWidth = 220, playButtonHeight = 50;
int planeWidth = 48, planeHeight = 48;
int inMenu = 1, gTimerID, AirplaneTimer;
time_t time1;
int number;
FILE *highf, *gamef;

int score = 0, highScore, hs;
char scoreC[20];

int numberOfPlanes = 0;


struct coordinate
{
	int type;
	int pic;
	float x, y;
	int numOfdests, ptr;
	float destX[1000], destY[1000];
	float degree;
	int isOut;
	float scale;
	int landed;
	float fuel;
	time_t t;
} plane[1000];



float distance(float x1, float y1, float x2, float y2);
void checkIfPlay(GtkWidget *widget, GdkEventButton *event);
gboolean nextFrame();
int draw(GtkWidget *widget, cairo_t *cr);

void genCoordinate();
void calcRadian(struct coordinate *pln);
void addToCoodinate(struct coordinate *pln);
void changeDest(struct coordinate *pln);

void mouseMove(GtkWidget *widget, GdkEventMotion *event);
void mouseRelease(GtkWidget *widget, GdkEventButton *event);

int inRedZone(struct coordinate *pln);
int inYellowZone(struct coordinate *pln);
int inHZone(struct coordinate *pln);

void load();
void save();


int main(int argc, char **argv)
{
	time1 = time(NULL);
	srand(time(0));
	setbuf(stdout, NULL);

	highf = fopen("highScore.txt", "r+");
	gamef = fopen("gameSave.txt", "r+");

	fscanf(highf, "%d", &highScore);
	hs = highScore;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	darea = gtk_drawing_area_new();

	gtk_widget_set_size_request(GTK_WIDGET(window), windowWidth, windowHeight);
	gtk_window_set_title(GTK_WINDOW(window), "Airplane");
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

	gtk_container_add(GTK_CONTAINER(window), darea);

	gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);
	gtk_widget_add_events(window, GDK_BUTTON_MOTION_MASK);
	gtk_widget_add_events(window, GDK_BUTTON_RELEASE_MASK);
	gtk_widget_add_events(window, GDK_POINTER_MOTION_MASK);


	g_signal_connect(G_OBJECT(window), "destroy",
		G_CALLBACK(gtk_main_quit), NULL);

	g_signal_connect(G_OBJECT(window), "button-press-event",
			G_CALLBACK(checkIfPlay), NULL);

	g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(draw), NULL);

	gtk_widget_set_app_paintable(window, TRUE);

	gtk_widget_show_all(window);

	gtk_main();


	if (highScore > hs)
	{
		rewind(highf);
		fprintf(highf, "%d\n", highScore);
	}

	fclose(highf);
	fclose(gamef);

	return 0;
}


void save()
{
	rewind(gamef);

	fprintf(gamef, "%d\n", score);
	fprintf(gamef, "%d\n", numberOfPlanes);

	int i, j;
	for (i = 0; i < numberOfPlanes; i++)
	{
		fprintf(gamef, "%d\n%d\n%f\n%f\n", plane[i].type, plane[i].pic, plane[i].x, plane[i].y);

		fprintf(gamef, "%d\n%d\n", plane[i].numOfdests, plane[i].ptr);
		for (j = 0; j < plane[i].numOfdests; j++)
			fprintf(gamef, "%f %f\n", plane[i].destX[j], plane[i].destY[j]);

		fprintf(gamef, "%f\n%d\n%f\n%d\n%f\n\n\n", plane[i].degree, plane[i].isOut, plane[i].scale, plane[i].landed, plane[i].fuel);
	}
}


void load()
{	
	rewind(gamef);
	fscanf(gamef, "%d", &score);
	fscanf(gamef, "%d", &numberOfPlanes);

	int i, j;
	for (i = 0; i < numberOfPlanes; i++)
	{
		fscanf(gamef, "%d%d", &plane[i].type, &plane[i].pic);
		fscanf(gamef, "%f%f", &plane[i].x, &plane[i].y);

		fscanf(gamef, "%d%d", &plane[i].numOfdests, &plane[i].ptr);
		for (j = 0; j < plane[i].numOfdests; j++)
			fscanf(gamef, "%f%f", &plane[i].destX[j], &plane[i].destY[j]);

		fscanf(gamef, "%f%d%f%d%f", &plane[i].degree, &plane[i].isOut, &plane[i].scale, &plane[i].landed, &plane[i].fuel);
		plane[i].t = time(NULL);
	}

	time1 = time(NULL);

}

float distance(float x1, float y1, float x2, float y2) 
{
	return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

void checkIfPlay(GtkWidget *widget, GdkEventButton *event)
{
	if (inMenu == 1)
	{
		if (event->x >= 150 &&
			event->x <= 370  )
			if (event->y >= 262 &&
				event->y <=332  )
				{
					if (gTimerID)
						g_source_remove(gTimerID);
					inMenu = 0;
					numberOfPlanes = 0;
					score = 0;
					gTimerID = g_timeout_add( 10.0, (GSourceFunc) nextFrame, NULL);
				}

		if (event->x >= 182 && event->x <= 328)
			if (event->y >= 478 && event->y <= 546)
				gtk_widget_destroy(GTK_WIDGET(window) );


		if (event->x >= 150  &&  event->x <= 370
			 &&  event->y >= 360  &&  event->y <= 430)
		{
			if (gTimerID)
				g_source_remove(gTimerID);
			load();
			inMenu = 0;
			gTimerID = g_timeout_add( 10.0, (GSourceFunc) nextFrame, NULL);
		}
	}

	else if( inMenu == 0)
	{
		if ( distance(event->x, event->y, 55, 570) <= 35 )
				inMenu = 2;
		int i;
		for (i = 0; i < numberOfPlanes; i++)
			if ( distance(plane[i].x, plane[i].y, event->x - planeWidth / 2, event->y - planeHeight / 2) < 50 )
			{
				number = i;

				plane[i].numOfdests = 0;
				plane[i].ptr = 0;

				g_signal_connect(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(mouseMove), NULL);
				g_signal_connect(G_OBJECT(widget), "button-release-event", G_CALLBACK(mouseRelease), NULL);
			}

		gtk_widget_queue_draw(darea);
	}

	else if (inMenu == 2)
	{
		if (event->x >= 386 && event->x <= 554)
			if (event->y >= 275 && event->y <= 323)
				inMenu = 0;
		if (event->x >= 422 && event->x <= 515)
			if (event->y >= 441 && event->y <= 484)
				inMenu = 1;

		if (event->x >= 386 && event->x <= 554)
			if (event->y >= 345 && event->y <= 393)
				save();		
	}
	else if (inMenu == 3)
	{	

		if (event->x >= 433 && event->x <= 538)
			if (event->y >= 380 && event->y <= 425)
			{
				inMenu = 1;
				numberOfPlanes = 0;
				score = 0;
			}
	}
}

void mouseMove(GtkWidget *widget, GdkEventMotion *event)
{
	int i = number;

	if (distance(event->x - planeWidth / 2, event->y - planeHeight / 2, plane[i].x, plane[i].y) < 20) 
		return;

	if ( plane[i].numOfdests == 0)
	{
		plane[i].destX[plane[i].numOfdests] = event->x - planeWidth / 2;
		plane[i].destY[plane[i].numOfdests] = event->y - planeHeight / 2;
		plane[i].numOfdests++;
	}
	else if ( distance(event->x, event->y, plane[i].destX[plane[i].numOfdests - 1], plane[i].destY[plane[i].numOfdests - 1])  >  2)
	{
		plane[i].destX[plane[i].numOfdests] = event->x - planeWidth / 2;
		plane[i].destY[plane[i].numOfdests] = event->y - planeHeight / 2;
		plane[i].numOfdests++;
	}

	gtk_widget_queue_draw(darea);
}

void mouseRelease(GtkWidget *widget, GdkEventButton *event)
{
	g_signal_handlers_disconnect_by_func(widget, G_CALLBACK(mouseMove), NULL);
	g_signal_handlers_disconnect_by_func(widget, G_CALLBACK(mouseRelease), NULL);


	gtk_widget_queue_draw(darea);
}


gboolean nextFrame()
{
	if( !numberOfPlanes )
	{
		if (difftime( time(NULL), time1 ) >= 2)
		{
			time1 = time(NULL);
			numberOfPlanes++;
			genCoordinate();
		}
	}
	else
	{
		if ( difftime( time(NULL), time1) >= 7 )
		{
			time1 = time(NULL);
			numberOfPlanes++;
			genCoordinate();
		}
	}


	gtk_widget_queue_draw(darea);

	return TRUE;
}


int draw(GtkWidget *widget, cairo_t *cr)
{
	if (score > highScore)
		highScore = score;

	int i, j;
	char remain[5];

	if (inMenu == 1)
	{
		cairo_surface_t *menu = cairo_image_surface_create_from_png("menu.png");

		cairo_set_source_surface(cr, menu, 0, 0);
		cairo_paint(cr);
		cairo_surface_destroy(menu);

		snprintf(scoreC, 10, "%d", highScore);

		cairo_set_source_rgba(cr, 0.555, 0.04, 0.04, 1);

		cairo_select_font_face(cr, "fantasy", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size(cr, 23);

		cairo_move_to(cr, 160, 44);
		cairo_show_text(cr, scoreC);
	}
	else if (inMenu == 0)
	{
		cairo_surface_t *board = cairo_image_surface_create_from_png("board.png");

		cairo_surface_t *redPlane = cairo_image_surface_create_from_png("redPlane.png");
		cairo_surface_t *yellowPlane = cairo_image_surface_create_from_png("yellowPlane.png");
		cairo_surface_t *helicopter1 = cairo_image_surface_create_from_png("helicopter1.png");
		cairo_surface_t *helicopter2 = cairo_image_surface_create_from_png("helicopter2.png");
		cairo_surface_t *helicopter3 = cairo_image_surface_create_from_png("helicopter3.png");

		cairo_surface_t *planeShadow = cairo_image_surface_create_from_png("planeShadow.png");
		cairo_surface_t *heli1Shadow = cairo_image_surface_create_from_png("heli1Shadow.png");
		cairo_surface_t *heli2Shadow = cairo_image_surface_create_from_png("heli2Shadow.png");
		cairo_surface_t *heli3Shadow = cairo_image_surface_create_from_png("heli3Shadow.png");

		cairo_surface_t *warning = cairo_image_surface_create_from_png("warning.png");

		cairo_set_source_surface(cr, board, 0, 0);
		cairo_paint(cr);

		cairo_surface_destroy(board);

		//Score
		snprintf(scoreC, 10, "%d", score);

		cairo_set_source_rgba(cr, 0, 0.39, 0, 1);

		cairo_select_font_face(cr, "fantasy", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size(cr, 25);

		cairo_move_to(cr, 895, 35);
		cairo_show_text(cr, scoreC);

		//highScore
		snprintf(scoreC, 10, "%d", highScore);
		cairo_move_to(cr, 133, 37);
		cairo_show_text(cr, scoreC);


		for (i = 0; i < numberOfPlanes; i++)
		{
			if (plane[i].scale <= 0.73)
				plane[i].isOut = 1;

			if (plane[i].isOut)
				continue;
			

			if ( plane[i].type != 2  &&  inRedZone(&plane[i]) && plane[i].landed == 0)
			{
				plane[i].landed = 1;
				plane[i].degree = -M_PI / 60 * 11;
				plane[i].ptr = plane[i].numOfdests;
				
				if (plane[i].type == 0)
					score++;
			}
			if ( plane[i].type != 2  &&  inYellowZone(&plane[i]) && plane[i].landed == 0)
			{
				plane[i].landed = 1;
				plane[i].degree = 0;
				plane[i].ptr = plane[i].numOfdests;
				
				if (plane[i].type == 1)
					score++;
			}
			if ( plane[i].type == 2  &&  inHZone(&plane[i]) && plane[i].landed == 0)
			{
				plane[i].landed = 1;
				score++;
			}

			if (plane[i].landed == 1)
	
				plane[i].scale -= 0.005;


			if (plane[i].landed == 0)
			{
				plane[i].fuel -= difftime( time(NULL), plane[i].t);
				plane[i].t = time(NULL);
				if (plane[i].fuel == 0)
					inMenu = 3;
				if (plane[i].fuel < 10)
				{

					snprintf(remain, 10, "%d", (int)plane[i].fuel);

					cairo_set_source_rgba(cr, 1, 0, 0, 1);

					cairo_select_font_face(cr, "fantasy", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
					cairo_set_font_size(cr, 18);

					cairo_move_to(cr, plane[i].x, plane[i].y);
					cairo_show_text(cr, remain);
				}
				else 
				{

					snprintf(remain, 10, "%d", (int)plane[i].fuel);

					cairo_set_source_rgba(cr, 0, 0, 0, 1);

					cairo_select_font_face(cr, "fantasy", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
					cairo_set_font_size(cr, 15);

					cairo_move_to(cr, plane[i].x, plane[i].y);
					cairo_show_text(cr, remain);
				}
						
			}

			//warning
			for (j = 0; j < numberOfPlanes; j++)
			{
				if (i == j || plane[i].landed == 1 || plane[j].landed == 1)
					continue;

				if ( distance(plane[i].x, plane[i].y, plane[j].x, plane[j].y) < 2.3 * planeWidth )
				{
					cairo_set_source_surface(cr, warning, plane[i].x, plane[i].y);
					cairo_paint(cr);
				}

			}
			// For Shadow
			cairo_translate(cr, plane[i].x + planeWidth/4 + (planeWidth/4 * (1 - plane[i].scale) * 3 )
							  , plane[i].y + planeHeight/4 + (planeHeight/4 * (1 - plane[i].scale) * 3 ) );
			cairo_rotate(cr, plane[i].degree);
			cairo_scale(cr, plane[i].scale, plane[i].scale);

			if (plane[i].type == 0 || plane[i].type == 1)
				cairo_set_source_surface(cr, planeShadow, -planeWidth/2, -planeHeight/2);
			else
			{
				if (plane[i].pic == 0)
					cairo_set_source_surface(cr, heli1Shadow, -planeWidth/2, -planeHeight/2);
				else if (plane[i].pic == 1)
					cairo_set_source_surface(cr, heli2Shadow, -planeWidth/2, -planeHeight/2);
				else
					cairo_set_source_surface(cr, heli3Shadow, -planeWidth/2, -planeHeight/2);

				plane[i].pic++;
				plane[i].pic %= 3;
			}

			cairo_paint(cr);

			cairo_identity_matrix(cr);

			// For Plane
			cairo_translate(cr, plane[i].x + planeWidth/2, plane[i].y + planeHeight/2);
			cairo_rotate(cr, plane[i].degree);
			cairo_scale(cr, plane[i].scale, plane[i].scale);

			if (plane[i].type == 0)
				cairo_set_source_surface(cr, redPlane, -planeWidth/2, -planeHeight/2);
			else if (plane[i].type == 1)
				cairo_set_source_surface(cr, yellowPlane, -planeWidth/2, -planeHeight/2);
			else
			{
				if (plane[i].pic == 0)
					cairo_set_source_surface(cr, helicopter1, -planeWidth/2, -planeHeight/2);
				else if (plane[i].pic == 1)
					cairo_set_source_surface(cr, helicopter2, -planeWidth/2, -planeHeight/2);
				else
					cairo_set_source_surface(cr, helicopter3, -planeWidth/2, -planeHeight/2);

				plane[i].pic++;
				plane[i].pic %= 3;
			}

			cairo_paint(cr);

			cairo_identity_matrix(cr);

			if (plane[i].landed == 0)
			{
				for (j = plane[i].ptr; j < plane[i].numOfdests; j++)
				{
					cairo_set_source_rgba(cr, 1, 1, 1, 1);
					cairo_arc(cr, plane[i].destX[j] + planeWidth / 2 , plane[i].destY[j] + planeHeight / 2, 4, 0, M_PI * 2);
					cairo_fill(cr);
				}
			}

			if ( !(plane[i].type == 2  &&  plane[i].scale < 1) )
				addToCoodinate(&plane[i]);

			if (plane[i].ptr < plane[i].numOfdests)
			{
				calcRadian(&plane[i]);

				if ( distance(plane[i].x, plane[i].y, plane[i].destX[plane[i].ptr], plane[i].destY[plane[i].ptr]) < 20 )
					plane[i].ptr++;
			}
		}	
			
			for (i = 0; i < numberOfPlanes; i++)
				for (j = i + 1; j < numberOfPlanes; j++)
					if ( plane[i].landed == 0 && plane[j].landed == 0 && distance(plane[i].x, plane[i].y, plane[j].x, plane[j].y) < 33 )
						inMenu = 3;
			

		cairo_surface_destroy(redPlane);
		cairo_surface_destroy(yellowPlane);
		cairo_surface_destroy(helicopter1);
		cairo_surface_destroy(helicopter2);
		cairo_surface_destroy(helicopter3);

		cairo_surface_destroy(planeShadow);
		cairo_surface_destroy(heli1Shadow);
		cairo_surface_destroy(heli2Shadow);
		cairo_surface_destroy(heli3Shadow);

		cairo_surface_destroy(warning);
	}
	else if (inMenu == 2)
	{	
		cairo_surface_t *board = cairo_image_surface_create_from_png("board.png");

		cairo_surface_t *redPlane = cairo_image_surface_create_from_png("redPlane.png");
		cairo_surface_t *yellowPlane = cairo_image_surface_create_from_png("yellowPlane.png");
		cairo_surface_t *helicopter1 = cairo_image_surface_create_from_png("helicopter1.png");
		cairo_surface_t *helicopter2 = cairo_image_surface_create_from_png("helicopter2.png");
		cairo_surface_t *helicopter3 = cairo_image_surface_create_from_png("helicopter3.png");

		cairo_surface_t *planeShadow = cairo_image_surface_create_from_png("planeShadow.png");
		cairo_surface_t *heli1Shadow = cairo_image_surface_create_from_png("heli1Shadow.png");
		cairo_surface_t *heli2Shadow = cairo_image_surface_create_from_png("heli2Shadow.png");
		cairo_surface_t *heli3Shadow = cairo_image_surface_create_from_png("heli3Shadow.png");

		cairo_surface_t *warning = cairo_image_surface_create_from_png("warning.png");

		cairo_surface_t *pause = cairo_image_surface_create_from_png("pause.png");

		cairo_set_source_surface(cr, board, 0, 0);
		cairo_paint(cr);

		cairo_set_source_surface(cr, board, 0, 0);
		cairo_paint(cr);
		cairo_surface_destroy(board);

		snprintf(scoreC, 10, "%d", score);

		cairo_set_source_rgba(cr, 0, 0.39, 0, 1);

		cairo_select_font_face(cr, "fantasy", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size(cr, 25);

		cairo_move_to(cr, 895, 35);
		cairo_show_text(cr, scoreC);


		for (i = 0; i < numberOfPlanes; i++)
		{
			if (plane[i].scale <= 0.73)
				plane[i].isOut = 1;

			if (plane[i].isOut)
				continue;


			if (plane[i].landed == 0)
			{
				char remain[5];
				
				plane[i].t = time(NULL);

				//itoa( (int)plane[i].fuel, remain, 10);
				snprintf(remain, 10, "%d", (int)plane[i].fuel);

				cairo_set_source_rgba(cr, 0, 0, 0, 1);

				cairo_select_font_face(cr, "fantasy", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
				cairo_set_font_size(cr, 15);

				cairo_move_to(cr, plane[i].x, plane[i].y);
				cairo_show_text(cr, remain);
			}

			//warning
			for (j = 0; j < numberOfPlanes; j++)
			{
				if (i == j || plane[i].landed == 1 || plane[j].landed == 1)
					continue;

				if ( distance(plane[i].x, plane[i].y, plane[j].x, plane[j].y) < 2.3 * planeWidth )
				{
					cairo_set_source_surface(cr, warning, plane[i].x, plane[i].y);
					cairo_paint(cr);
				}

			}

			// For Shadow
			cairo_translate(cr, plane[i].x + planeWidth/4 + (planeWidth/4 * (1 - plane[i].scale) * 3 )
							  , plane[i].y + planeHeight/4 + (planeHeight/4 * (1 - plane[i].scale) * 3 ) );
			cairo_rotate(cr, plane[i].degree);
			cairo_scale(cr, plane[i].scale, plane[i].scale);

			if (plane[i].type == 0 || plane[i].type == 1)
				cairo_set_source_surface(cr, planeShadow, -planeWidth/2, -planeHeight/2);
			else
				cairo_set_source_surface(cr, heli1Shadow, -planeWidth/2, -planeHeight/2);

			cairo_paint(cr);

			cairo_identity_matrix(cr);

			// For Plane
			cairo_translate(cr, plane[i].x + planeWidth/2, plane[i].y + planeHeight/2);
			cairo_rotate(cr, plane[i].degree);
			cairo_scale(cr, plane[i].scale, plane[i].scale);

			if (plane[i].type == 0)
				cairo_set_source_surface(cr, redPlane, -planeWidth/2, -planeHeight/2);
			else if (plane[i].type == 1)
				cairo_set_source_surface(cr, yellowPlane, -planeWidth/2, -planeHeight/2);
			else
				cairo_set_source_surface(cr, helicopter1, -planeWidth/2, -planeHeight/2);

			cairo_paint(cr);

			cairo_identity_matrix(cr);

			if (plane[i].landed == 0)
			{
				for (j = plane[i].ptr; j < plane[i].numOfdests; j++)
				{
					cairo_set_source_rgba(cr, 1, 1, 1, 1);
					cairo_arc(cr, plane[i].destX[j] + planeWidth / 2 , plane[i].destY[j] + planeHeight / 2, 4, 0, M_PI * 2);
					cairo_fill(cr);
				}
			}
		}

		cairo_set_source_surface(cr, pause, 280,112 );
		cairo_paint(cr);


		cairo_surface_destroy(pause);
		cairo_surface_destroy(redPlane);
		cairo_surface_destroy(yellowPlane);
		cairo_surface_destroy(helicopter1);
		cairo_surface_destroy(helicopter2);
		cairo_surface_destroy(helicopter3);

		cairo_surface_destroy(planeShadow);
		cairo_surface_destroy(heli1Shadow);
		cairo_surface_destroy(heli2Shadow);
		cairo_surface_destroy(heli3Shadow);

		cairo_surface_destroy(warning);

	}
	else if (inMenu == 3)
	{	
		cairo_surface_t *board = cairo_image_surface_create_from_png("board.png");

		cairo_surface_t *redPlane = cairo_image_surface_create_from_png("redPlane.png");
		cairo_surface_t *yellowPlane = cairo_image_surface_create_from_png("yellowPlane.png");
		cairo_surface_t *helicopter1 = cairo_image_surface_create_from_png("helicopter1.png");
		cairo_surface_t *helicopter2 = cairo_image_surface_create_from_png("helicopter2.png");
		cairo_surface_t *helicopter3 = cairo_image_surface_create_from_png("helicopter3.png");

		cairo_surface_t *planeShadow = cairo_image_surface_create_from_png("planeShadow.png");
		cairo_surface_t *heli1Shadow = cairo_image_surface_create_from_png("heli1Shadow.png");
		cairo_surface_t *heli2Shadow = cairo_image_surface_create_from_png("heli2Shadow.png");
		cairo_surface_t *heli3Shadow = cairo_image_surface_create_from_png("heli3Shadow.png");

		cairo_surface_t *warning = cairo_image_surface_create_from_png("warning.png");

		cairo_surface_t *over = cairo_image_surface_create_from_png("over.png");

		cairo_set_source_surface(cr, board, 0, 0);
		cairo_paint(cr);

		cairo_set_source_surface(cr, board, 0, 0);
		cairo_paint(cr);
		cairo_surface_destroy(board);

		snprintf(scoreC, 10, "%d", score);

		cairo_set_source_rgba(cr, 0, 0.39, 0, 1);

		cairo_select_font_face(cr, "fantasy", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size(cr, 25);

		cairo_move_to(cr, 895, 35);
		cairo_show_text(cr, scoreC);


		for (i = 0; i < numberOfPlanes; i++)
		{
			if (plane[i].scale <= 0.73)
				plane[i].isOut = 1;

			if (plane[i].isOut)
				continue;

			if (plane[i].landed == 0)
			{
				char remain[5];
				
				plane[i].t = time(NULL);

				//itoa( (int)plane[i].fuel, remain, 10);
				snprintf(remain, 10, "%d", (int)plane[i].fuel);

				cairo_set_source_rgba(cr, 0, 0, 0, 1);

				cairo_select_font_face(cr, "fantasy", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
				cairo_set_font_size(cr, 15);

				cairo_move_to(cr, plane[i].x, plane[i].y);
				cairo_show_text(cr, remain);
			}

			//warning
			for (j = 0; j < numberOfPlanes; j++)
			{
				if (i == j || plane[i].landed == 1 || plane[j].landed == 1)
					continue;

				if ( distance(plane[i].x, plane[i].y, plane[j].x, plane[j].y) < 2.3 * planeWidth )
				{
					cairo_set_source_surface(cr, warning, plane[i].x, plane[i].y);
					cairo_paint(cr);
				}

			}


			// For Shadow
			cairo_translate(cr, plane[i].x + planeWidth/4 + (planeWidth/4 * (1 - plane[i].scale) * 3 )
							  , plane[i].y + planeHeight/4 + (planeHeight/4 * (1 - plane[i].scale) * 3 ) );
			cairo_rotate(cr, plane[i].degree);
			cairo_scale(cr, plane[i].scale, plane[i].scale);

			if (plane[i].type == 0 || plane[i].type == 1)
				cairo_set_source_surface(cr, planeShadow, -planeWidth/2, -planeHeight/2);
			else
				cairo_set_source_surface(cr, heli1Shadow, -planeWidth/2, -planeHeight/2);

			cairo_paint(cr);

			cairo_identity_matrix(cr);

			// For Plane
			cairo_translate(cr, plane[i].x + planeWidth/2, plane[i].y + planeHeight/2);
			cairo_rotate(cr, plane[i].degree);
			cairo_scale(cr, plane[i].scale, plane[i].scale);

			if (plane[i].type == 0)
				cairo_set_source_surface(cr, redPlane, -planeWidth/2, -planeHeight/2);
			else if (plane[i].type == 1)
				cairo_set_source_surface(cr, yellowPlane, -planeWidth/2, -planeHeight/2);
			else
				cairo_set_source_surface(cr, helicopter1, -planeWidth/2, -planeHeight/2);

			cairo_paint(cr);

			cairo_identity_matrix(cr);

			if (plane[i].landed == 0)
			{
				for (j = plane[i].ptr; j < plane[i].numOfdests; j++)
				{
					cairo_set_source_rgba(cr, 1, 1, 1, 1);
					cairo_arc(cr, plane[i].destX[j] + planeWidth / 2 , plane[i].destY[j] + planeHeight / 2, 4, 0, M_PI * 2);
					cairo_fill(cr);
				}
			}
		}
		

		cairo_set_source_surface(cr, over, 280,112 );
		cairo_paint(cr);


		cairo_surface_destroy(over);
		cairo_surface_destroy(redPlane);
		cairo_surface_destroy(yellowPlane);
		cairo_surface_destroy(helicopter1);
		cairo_surface_destroy(helicopter2);
		cairo_surface_destroy(helicopter3);

		cairo_surface_destroy(planeShadow);
		cairo_surface_destroy(heli1Shadow);
		cairo_surface_destroy(heli2Shadow);
		cairo_surface_destroy(heli3Shadow);

		cairo_surface_destroy(warning);
		
	}
	return 0;
}



void genCoordinate()
{
	int n = numberOfPlanes - 1;
	plane[n].type = rand() % 3;
	plane[n].pic = 0;
	plane[n].scale = 1;
	plane[n].landed = 0;
	plane[n].t = time(NULL);

	plane[n].fuel = 61;
	if (plane[n].type == 2)
		plane[n].fuel = 42;

	int side = rand() % 4;

	if (side == 0)   	 // Start from Left
	{
		plane[n].x = -50;
		plane[n].y = rand() % (windowHeight / 2);
		plane[n].y += 150;

		plane[n].destX[0] = windowWidth + 200;
		plane[n].destY[0] = rand() % (300 + windowHeight);
		plane[n].destY[0] -= 150;
	}
	else if (side == 1)  // Start from Up
	{
		plane[n].y = -50;
		plane[n].x = rand() % (windowWidth / 2);
		plane[n].x += 250;

		plane[n].destY[0] = windowHeight + 200;
		plane[n].destX[0] = rand() % (500 + windowWidth);
		plane[n].destX[0] -= 250;
	}
	else if (side == 2)  // Start from Right 
	{
		plane[n].x = windowWidth + 50;
		plane[n].y = rand() % (windowHeight / 2);
		plane[n].y += 150;

		plane[n].destX[0] = -200;
		plane[n].destY[0] = rand() % (300 + windowHeight);
		plane[n].destY[0] -= 150;
	}
	else				 // Start from Down
	{
		plane[n].y = windowHeight + 50;
		plane[n].x = rand() % (windowWidth / 2);
		plane[n].x += 250;

		plane[n].destY[0] = -200;
		plane[n].destX[0] = rand() % (500 + windowWidth);
		plane[n].destX[0] -= 250;
	}

	int i;
	for (i = 0; i < n; i++)
		if ( distance(plane[n].x, plane[n].y, plane[i].x, plane[i].y) <= 100 )
		{
			genCoordinate();
			return;
		}

	plane[n].numOfdests = 1;
	plane[n].ptr = 0;
	
	calcRadian(&plane[n]);
	plane[n].ptr++;
}


void calcRadian(struct coordinate *pln)
{
	float tmp;
	int a = pln->destY[pln->ptr] - pln->y;
	int b = pln->destX[pln->ptr] - pln->x;

	if (b == 0)
	{
		
		if (a == 0)
		{
			pln->degree = 0;
			pln->ptr++;
			return;
		}
		else if ( a > 0 )
		{
			pln->degree = M_PI / 2;
			pln->ptr++;
			return;
		}
		else
		{
			pln->degree = M_PI / -2;
			pln->ptr++;
			return;
		}
	}

	tmp = (float)a / b;
	tmp = atan(tmp);

	if (b <= 0)
		tmp += M_PI;

	pln->degree = tmp;
}


void addToCoodinate(struct coordinate *pln)
{
	float speed = 2.1;
	if(pln->type == 2)
		speed = 1.3;

	pln->x += cos(pln->degree) * speed;
	pln->y += sin(pln->degree) * speed;

	if (pln->x >= -50     &&   pln->x <= windowWidth + 50   &&  pln->y >= -50   &&   pln->y <= windowHeight + 50)
		return;

	pln->isOut = 1;
	inMenu = 3;


}
int inRedZone(struct coordinate *pln)
{
	int chk = 0;

	int px = pln->x + planeWidth / 2;
	int py = pln->y + planeHeight / 2;

	if ( px >= 515  &&  px <= 550  &&  py >= 350  &&  py <= 380)
		chk = 1;

	if ( !(pln->degree <= -M_PI / 18   &&   pln->degree >= -M_PI / 4)  )
		chk = 0;

	return chk;
}


int inYellowZone(struct coordinate *pln)
{
	int chk = 0;

	int px = pln->x + planeWidth / 2;
	int py = pln->y + planeHeight / 2;

	if ( px >= 630  &&  px <= 670  &&  py >= 175  &&  py <= 210)
		chk = 1;

	if ( !(pln->degree >= -M_PI / 9   &&   pln->degree <= M_PI / 9)  )
		chk = 0;

	return chk;
}


int inHZone(struct coordinate *pln)
{
	int px = pln->x + planeWidth / 2;
	int py = pln->y + planeHeight / 2;

	if ( distance(px, py, 692, 342) <= 20 )
		return 1;
	else
		return 0;
}
