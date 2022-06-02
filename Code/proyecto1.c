#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <SDL2/SDL.h>


#define WIDTH 1080
#define HEIGHT 1080

/* Comando para transformar formatos de imagen:  anytopnm car.jpg | pnmtoplainpnm > car.ppm */

//////////// Declaraciones de SDL ////////////

typedef struct{
	SDL_Renderer *render;
	SDL_Window *window;
	int modo;
	int operacion;
	int firma;
}App;

////////////////////////////////////////////


typedef struct{ 
	int x0;
	int y0;
	int x1;
	int y1;
	int y_max; //Parametros para pintar por scanline, "y" max de cada borde
	int y_min;
	int activo;
	double m;
	double delta_m;
	int x_anterior;
}Bordes;

typedef struct {
	Bordes *bordes_ord; // Bordes ordenados de mayor a menor segun el valor de y_max
	
	int cantidad;       // Cantidad de bordes del poligono
	int max_y;          // Valor maximo en "y" del poligono
	int min_y;
}pintar;

typedef struct interseccion{
	int int_x; // punto x de la interseccion
	int int_y; // Punto y de la interseccion
	int min_b; // Valor minimo del borde en y
	int max_b; // Valor maximo del borde en y
	int cant;
}interseccion;

typedef struct intersec_partes{
	struct interseccion* intersecciones; // Guardamos las intersecciones correspondientes a cada poligono o parte del carro
	int cant; // Guardamos la cantidad de intersecciones.
	int pintado;
}intersec_partes;

// Declaracion de estructuras para las partes del carro //
/*Estructura para pintado de unicamente bordes*/
Bordes *bumper_t;
Bordes *foco_t;
Bordes *guarda_t;
Bordes *carroceria;
Bordes *cola;
Bordes *llanta_t;
Bordes *parab_t;
Bordes *parab_d;
Bordes *guarda_d;
Bordes *llanta_d;
Bordes *foco_d;
Bordes *ventana;
Bordes *div_w;
Bordes *retro;
Bordes *detalle1;
Bordes *detalle2;
Bordes *detalle3;
Bordes *detalle4;

/*Partes pintadas (relleno)*/
pintar *bumper_tp;
pintar *foco_tp;
pintar *guarda_tp;
pintar *carroceriap;
pintar *colap;
pintar *llanta_tp;
pintar *parab_tp;
pintar *parab_dp;
pintar *guarda_dp;
pintar *llanta_dp;
pintar *foco_dp;
pintar *ventana_p;
pintar *div_p;
pintar *retro_p;
pintar *detalle1_p;
pintar *detalle2_p;
pintar *detalle3_p;
pintar *detalle4_p;

/* Intersecciones para pintado */
intersec_partes *bumper_ti;
intersec_partes *foco_ti;
intersec_partes *guarda_ti;
intersec_partes *carroceriai;
intersec_partes *colai;
intersec_partes *llanta_ti;
intersec_partes *parab_ti;
intersec_partes *parab_di;
intersec_partes *guarda_di;
intersec_partes *llanta_di;
intersec_partes *foco_di;
intersec_partes *ventana_pi;
intersec_partes *div_pi;
intersec_partes *retro_pi;
intersec_partes *detalle1_pi;
intersec_partes *detalle2_pi;
intersec_partes *detalle3_pi;
intersec_partes *detalle4_pi;
/////////////////////////////////////////////////////////////

App app;
FILE *coord;
int bordes_activos=0; // Variable contadora de los bordes activos con respecto a un scanline
int aux_t = 0;

int panx_g;
int pany_g;
float escalx_g;
float escaly_g;
float angulo;

int TH, TW; //Alto y ancho de la imagen
int **R,**G,**B;

int *X,*Y; 				// Matrices globales para guardar los puntos de bezier
int puntos_bezier = 0; //Cantidad de puntos de bezier
int pintar_mouse = 0;
int x_before = 0, y_before = 0;

void leer_imagen();
void set_color_texture(int x, int scanline, int TH_I, int TW_I);
void plot_pixel(int x, int y);
void curvas_Bezier(int x[], int y[]);
void leer_firma();


void init_SDL(){
	SDL_Init(SDL_INIT_VIDEO);
	app.window = SDL_CreateWindow("Graficos 2D", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,WIDTH, HEIGHT, 0);
	app.render = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_ACCELERATED);
}

void clean_window(void){
	SDL_SetRenderDrawColor(app.render, 0, 170, 228, 0);
	SDL_RenderClear(app.render);
	SDL_RenderPresent(app.render); // Para limpiar la ventana
}

void keydown(SDL_KeyboardEvent *event){
	if(event-> repeat == 0 ){
		if(event->keysym.scancode == SDL_SCANCODE_1){ //Para modo 1
			app.modo = 1;
		}
		if(event->keysym.scancode == SDL_SCANCODE_2){ //Para modo 2
			app.modo = 2;
		}
		if(event->keysym.scancode == SDL_SCANCODE_3){ //Para modo 3
			app.modo = 3;
		}
		if(event->keysym.scancode == SDL_SCANCODE_4){ //Para modo 4
			app.modo = 4;
		}

		if(event->keysym.scancode == SDL_SCANCODE_F){ //Para realizar firma
			app.firma = 1;
		}

		if(event->keysym.scancode == SDL_SCANCODE_B){ //Para realizar firma con mouse
			app.firma = 2;
		}

		if(event->keysym.scancode == SDL_SCANCODE_Z){ //Para modo 4
			app.operacion = 1;
		}
		if(event->keysym.scancode == SDL_SCANCODE_X){ //Para modo 4
			app.operacion = 2;
		}

		if(event->keysym.scancode == SDL_SCANCODE_UP){ //Para pan Arriba
			app.operacion = 3;
		}

		if(event->keysym.scancode == SDL_SCANCODE_DOWN){  //Para pan abajo
			app.operacion = 4;
		}
		if(event->keysym.scancode == SDL_SCANCODE_LEFT){  //Para pan izquierda
			app.operacion = 5;
		}
		if(event->keysym.scancode == SDL_SCANCODE_RIGHT){  //Para pan derecha
			app.operacion = 6;
		}
		if(event->keysym.scancode == SDL_SCANCODE_R){  //Para resetear
			app.operacion = 7;
		}
		// if(event->keysym.scancode == SDL_SCANCODE_A){  //Para resetear
		// 	app.operacion = 8;
		// }
		// if(event->keysym.scancode == SDL_SCANCODE_D){  //Para resetear
		// 	app.operacion = 9;
		// }

	} 
}

void keyup(SDL_KeyboardEvent *event){
	if(event-> repeat == 0 ){
		if(event->keysym.scancode == SDL_SCANCODE_Z){ //Para modo 4
			app.operacion = 0;
		}
		if(event->keysym.scancode == SDL_SCANCODE_X){ //Para modo 4
			app.operacion = 0;
		}
		if(event->keysym.scancode == SDL_SCANCODE_UP){ //Para modo 4
			app.operacion = 0;
		}
		if(event->keysym.scancode == SDL_SCANCODE_DOWN){ //Para modo 4
			app.operacion = 0;
		}
		if(event->keysym.scancode == SDL_SCANCODE_LEFT){ //Para modo 4
			app.operacion = 0;
		}
		if(event->keysym.scancode == SDL_SCANCODE_RIGHT){ //Para modo 4
			app.operacion = 0;
		}
		if(event->keysym.scancode == SDL_SCANCODE_R){ //Para resetear
			app.operacion = 0;
		}
		// if(event->keysym.scancode == SDL_SCANCODE_A){  //Para resetear
		// 	app.operacion = 0;
		// }
		// if(event->keysym.scancode == SDL_SCANCODE_D){  //Para resetear
		// 	app.operacion = 0;
		// }

	}
}

void input(void){ // Para gestionar entradas o salidas de SDL
	SDL_Event event;

	while(SDL_PollEvent(&event)){
		
		switch(event.type){
			case SDL_QUIT:
				exit(0);
				break;

			case SDL_KEYDOWN: //cuando una tecla es presionada
				keydown(&event.key);
				break;

			case SDL_KEYUP: // cuando alguna tecla deja de ser presionada
				keyup(&event.key);
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(event.button.button == SDL_BUTTON_LEFT){
					if(puntos_bezier < 4){
						X[puntos_bezier]= event.button.x;
						Y[puntos_bezier] = event.button.y;
						puntos_bezier++;
					}
					
				}
				break;


			case SDL_MOUSEBUTTONUP:
				if(event.button.button == SDL_BUTTON_LEFT){
					// mouse_x= event.button.x;
					// mouse_y = event.button.y;
				}
				break;

			// case SDL_MOUSEMOTION:
			// 	mouse_x = event.motion.x;
			// 	mouse_y = event.motion.y;
			// 	break;

			default:
				break;
		}

	}
}

/* Funcion para dibujar pixel */
void plot_pixel(int x, int y){
	if(app.modo == 3 || app.modo == 4){
		set_color_texture(x,y,TH, TW);
	}
	
	SDL_RenderDrawPoint(app.render, x, y);
}

/* Algoritmo de Bresenham para el octante 1*/
void bresenham(int x0, int y0, int x1, int y1){
	

	//Primer cuadrante
	if((x0<=x1)&&(y0<=y1)){
		
		if(abs(x0-x1)>=abs(y0-y1)){ //PRIMER OCTANTE

			int delta_E, delta_NE, d, xp, yp;

			delta_E = 2*(y1-y0); 
			delta_NE = 2*((y1-y0) - (x1-x0));

			xp = x0;
			yp = y0;

			plot_pixel(xp,yp);  

			d= 2*(y1-y0) - (x1-x0); // d incial

			while(xp<x1){
				if(d<=0){ // pintar E
					xp++;
					d = d + delta_E;
				}
				else{ //pintar NE
					xp++;
					yp++;
					d = d + delta_NE;
				}
				plot_pixel(xp,yp);
			}

		}


		else { //SEGUNDO OCTANTE

			int delta_N, delta_NE, d, xp, yp;

			delta_NE = 2*((y1-y0) - (x1-x0));
			delta_N = 2*(x0-x1); 

			xp = x0;
			yp = y0;

			plot_pixel(xp,yp);

			d= (y1-y0) - 2*(x1-x0); // d incial

			while(yp<y1){
				if(d<=0){ // pintar NE
					xp++;
					yp++;
					d = d + delta_NE;
				}
				else{ //pintar N
					yp++;
					d = d + delta_N;
				}
				plot_pixel(xp,yp);
			}
		}
	}

	//Segundo cuadrante
	else if((x0>x1)&&(y0<y1)){

		if(abs(x0-x1) <= abs(y1-y0)){  // TERCER OCTANTE
			int delta_N, delta_NO, d, xp, yp;

			delta_NO = 2*((y0-y1) - (x1-x0));
			delta_N = 2*(x0-x1); 

			xp = x0;
			yp = y0;

			plot_pixel(xp,yp);  

			d= (y0-y1) - 2*(x1-x0); 

			while(yp<y1){
				if(d<=0){ // pintar NO
					yp++;
					d = d + delta_N;

				}
				else{ //pintar N
					xp--;
					yp++;
					d = d + delta_NO;
				}
				plot_pixel(xp,yp);
			}
		}

		else {   //CUARTO OCTANTE
			int delta_O, delta_NO, d, xp, yp;

			delta_O = 2*(y0-y1); 
			delta_NO = 2*((y0-y1) - (x1-x0));

			xp = x0;
			yp = y0;

			plot_pixel(xp,yp); 

			d= 2*(y0-y1) - (x1-x0); 

			while(xp>x1){
				if(d<=0){ // pintar NO
					xp--;
					yp++;
					d = d + delta_NO;
				}
				else{ //pintar O
					xp--;
					d = d + delta_O;
				}
				plot_pixel(xp,yp);
			}

		}

	}

	// Tercer cuadrante
	else if((x0>x1)&&(y0>=y1)){

		if(abs(x1-x0)>=abs(y1-y0)){   // QUINTO OCTANTE
			int delta_O, delta_SO, d, xp, yp;

			delta_O = 2*(y0-y1); 
			delta_SO = 2*((y0-y1) + (x1-x0));

			xp = x0;
			yp = y0;

			plot_pixel(xp,yp);

			d= 2*(y0-y1) + (x1-x0); 

			while(xp>x1){
				if(d<=0){ // pintar O
					xp--;
					d = d + delta_O;
					
				}
				else{ //pintar SO
					xp--;
					yp--;
					d = d + delta_SO;
				}
				plot_pixel(xp,yp);
			}
		}
		else {    // SEXTO OCTANTE
			int delta_S, delta_SO, d, xp, yp;

			delta_S = 2*(x1-x0); 
			delta_SO = 2*((y0-y1) + (x1-x0));

			xp = x0;
			yp = y0;

			plot_pixel(xp,yp); 

			d= (y0-y1) + 2*(x1-x0); 

			while(yp>y1){
				if(d<=0){ // pintar SO
					xp--;
					yp--;
					d = d + delta_SO;
					
				}
				else{ //pintar S
					yp--;
					d = d + delta_S;
				}
				plot_pixel(xp,yp);
			}
		}

	}

	// Cuarto cuadrante
	else if ((x0<=x1)&&(y0>=y1)){
		if(abs(x1-x0) <= abs(y1-y0)){ //SEPTIMO OCTANTE
			int delta_S, delta_SE, d, xp, yp;

			delta_S = 2*(x1-x0); 
			delta_SE = 2*((y1-y0) + (x1-x0));

			xp = x0;
			yp = y0;

			plot_pixel(xp,yp); 

			d= (y1-y0) + 2*(x1-x0); 

			while(yp>y1){
				if(d<=0){ // pintar SO
					yp--;
					d = d + delta_S;
					
				}
				else{ //pintar SE
					xp++;
					yp--;
					d = d + delta_SE;
					
				}
				plot_pixel(xp,yp);
			}
		}

		else { // OCTAVO OCTANTE
			int delta_E, delta_SE, d, xp, yp;

			// estas son constantes //
			delta_E = 2*(y1-y0);
			delta_SE = 2*((y1-y0) + (x1-x0));

			xp = x0;
			yp = y0;

			plot_pixel(xp,yp);  // se pinta el primer pixel

			d= 2*(y1-y0) + (x1-x0); // d incial

			/** Debido a que este analisis es para el primer octante entonces x simpre es mayor que y 
			 *  por lo que no se toma en cuenta la densidad **/

			while(xp<x1){
				if(d<=0){ // pintar SE
					xp++;
					yp--;
					d = d + delta_SE;
					
				}
				else{ //pintar E
					xp++;
					d = d + delta_E;
							
				}
				plot_pixel(xp,yp);
			}

		}


	}

}

Bordes* espacio_partes(){  //Funcion donde se inicializa y reserva espacio para la estructura "Bordes"
	Bordes *x;
	x = (Bordes *)malloc(900 * sizeof(Bordes)); //450 es un maximo de vertices de los poligonos
	
	for(int i=0; i<900; i++){		// Se carga 0 por default
		x[i].x0=0;
		x[i].y0=0;
		x[i].x1=0;
		x[i].y1=0;
		x[i].y_max = 0;
		x[i].y_min = 0;
		x[i].m = 0;
		x[i].delta_m = 0;
		x[i].activo = 0;
		x[i].x_anterior=0;
	}					
		
	return x;
}

pintar* espacio_pintado(){  //Funcion para inicializar y reservar espacio para la estructura "pintar"
	pintar* x;
	Bordes* b = (Bordes *)malloc(900*sizeof(Bordes));
	x=(pintar *)malloc(sizeof(pintar));
	x->bordes_ord=b;

	//Para inicializar en 0 el array de coordenadas ordenadas previamente
	for(int i = 0; i<900; i++ ){
		x->bordes_ord[i].x0=0;
		x->bordes_ord[i].y0=0;
		x->bordes_ord[i].x1=0;
		x->bordes_ord[i].y1=0;
		x->bordes_ord[i].y_max=0;
		x->bordes_ord[i].y_min = 0;
	}
	x->cantidad=0;
	x->max_y=0;
	x->min_y=0;
	return x;
}

void vertices_polig(float escaly,float escalx, int pan_y, int pan_x){  // Funcion para extraer coordenadas del archivo y ordenar coordenadas para scanline
	
	rewind(coord); 		//Puntero del archivo se coloca al inicio
	Bordes* vertice; 	// Se guardaran las coordenadas de los poligonos
	Bordes* vertice_aux;
	Bordes* carro[18] = {bumper_t, foco_t, guarda_t, cola, carroceria,llanta_t, parab_t,parab_d,guarda_d,llanta_d,foco_d, ventana, div_w, retro,detalle1,detalle2,detalle3,detalle4};
	pintar* pintar_aux; // Para guardar los datos de cada poligono
	pintar* carro_pintado[18] = {bumper_tp, foco_tp, guarda_tp, colap, carroceriap,llanta_tp, parab_tp,parab_dp,guarda_dp,llanta_dp,foco_dp,ventana_p,div_p,retro_p,detalle1_p,detalle2_p,detalle3_p,detalle4_p};

	int part = 0; 	 // Para recorrer la lista carro[]
	int i=0; 	 	 // Contador de las rectas que forman los vertices
	int x0=0; int y0=0;
	int x1=0; int y1=0;
	int max_pol = -1; int min_pol = 2000; //Variables para escoger el "y" minimo y "y" maximo de un poligono 
	int cant = 0; 						//variable para cantidades de bordes que  se presentan en el poligono

	fscanf(coord, "%d,%d", &x0, &y0); //Se carga primero las coordenadas primeras del archivo
	x0=(int)x0;
	y0=(int)y0;
	int refx = (WIDTH/2)-1;
	int refy = 356;


	vertice = espacio_partes();   //inicializamos la estructuras vertice y vertice_aux
	vertice_aux = espacio_partes();
	pintar_aux = espacio_pintado();

	/* Creamos un ciclo donde se recorra el archivo con las coordenadas de los poligonos
	   y se van gaurdando en vertice y vertice_aux*/
	while(!feof(coord) && part < 18){
		
		//printf("Iniciales (%i, %i)", x0,y0);
		if((int)x0 != 0 && (int)y0 != 0){ 		// Corrobora si no se ha llegado al final de ese poligono

			fscanf(coord, "%d,%d", &x1, &y1); // Leemos el siguiente vertice
			x1=(int)x1;
			y1=(int)y1;

			//printf("  Finales (%i, %i) \n", x1,y1);

			if((int)x1 != 0 && (int)y1 != 0){ //Comprob si no se ha llegado al final del poligono
				vertice[i].x0=x0;
				vertice[i].y0=y0;
				vertice[i].x1=x1;
				vertice[i].y1=y1;
				vertice_aux[i].x0=x0; vertice_aux[i].y0=y0; vertice_aux[i].x1=x1; vertice_aux[i].y1=y1;

				if(y0>y1){
					vertice_aux[i].y_max = y0;
					vertice_aux[i].y_min = y1; 

					//Aqui escogemos el maximo y el minimo en "y" para el poligono en cuestion 
					if(y0>max_pol)
						max_pol=y0;
					
					if(y1<min_pol){
						min_pol=y1;
					}
				}
				else if(y0<=y1){
					vertice_aux[i].y_max = y1;
					vertice_aux[i].y_min = y0;
					//printf("max: %i  min:%i", vertice_aux[i].y_max, vertice_aux[i].y_min);

					//Aqui escogemos el maximo y el minimo en "y" para el poligono en cuestion 
					if(y1>max_pol)
						max_pol=y1;
					
					if(y0<min_pol){
						min_pol=y0;
					}
				}

				// Actualizamos el vertice inicial de la siguiente recta //
				x0=x1; y0=y1;
				cant ++;
				i++;
			}

			else if((int)x1 == 0 && (int)y1 == 0){ // Cuando detecta final del poligono

				fscanf(coord, "%d,%d", &x0, &y0); //brinca a siguiente coordenada
				x0=(int)x0;
				y0=(int)y0;
				i = 0; 

		
				/* Aqui podemos hacer el proceso de escalamiento para cada una de los vertices del poligono */
				// Trasladamos-rotamos - trasladamos
				// Luego se requiere realizar el mapeo a frame buffer

				
				if((escaly != 1 && escalx != 1) || (pan_y != 0)){
					int xi, yi, xf, yf;
					int y_maxf, y_minf;

					/* Para Traslado y escala*/
					for(int g = 0; g<cant; g++){
						//trasladamos un punto arbitrario al origen y se hace con respecto a coordenada con 'y' minimo //
						xi = vertice[g].x0 - refx;
						yi = vertice[g].y0 - refy;
						xf = vertice[g].x1 - refx;
						yf = vertice[g].y1 - refy;
						y_maxf = vertice[g].y_max - refy;
						y_minf = vertice[g].y_min - refy;

						// Escalamos //
						xi = xi*escalx;
						yi = yi*escaly;
						xf = xf*escalx;
						yf = yf*escaly;
						y_maxf = y_maxf * escaly;
						y_minf = y_minf * escaly;

						// Volvemos a posicion original //
						vertice[g].x0 = xi + refx + (pan_x * escalx);
						vertice[g].y0 = yi + refy + (pan_y * escaly);
						vertice[g].x1 = xf + refx + (pan_x*escalx );
						vertice[g].y1 = yf + refy + (pan_y * escaly);
						vertice[g].y_max = y_maxf + refy +(pan_y * escaly);
						vertice[g].y_min = y_minf + refy +(pan_y * escaly);

						// Se realiza lo mismo pero para el auxiliar utilizado para partes_pintadas //
						//Trasladamos
						xi = vertice_aux[g].x0 - refx;
						yi = vertice_aux[g].y0 - refy;
						xf = vertice_aux[g].x1 - refx;
						yf = vertice_aux[g].y1 - refy;
						y_maxf = vertice_aux[g].y_max - refy;
						y_minf = vertice_aux[g].y_min - refy;

						// Escalamos //
						xi = xi*escalx;
						yi = yi*escaly;
						xf = xf*escalx;
						yf = yf*escaly;
						y_maxf = y_maxf * escaly;
						y_minf = y_minf * escaly;

						// Volvemos a posicion original //
						vertice_aux[g].x0 = xi + refx + (pan_x * escalx);
						vertice_aux[g].y0 = yi + refy + (pan_y * escaly);
						vertice_aux[g].x1 = xf + refx + (pan_x * escalx);
						vertice_aux[g].y1 = yf + refy+(pan_y * escaly);
						vertice_aux[g].y_max = y_maxf + refy+(pan_y * escaly);
						vertice_aux[g].y_min = y_minf + refy+(pan_y * escaly);
					}
				}


				carro[part] = vertice; // Aqui se guardan los vertices del archivo en memoria
				
				int maxr1 = max_pol - refy;
				int minr2=min_pol - refy;

				maxr1 = maxr1*escaly;
				minr2 = minr2*escaly;
				pintar_aux->max_y=maxr1 + refy + (pan_y * escaly);
				pintar_aux->min_y=minr2 + refy + (pan_y * escaly);

				pintar_aux->cantidad=cant;
				

				// Ordenamos los bordes de mayor "y" a un "y" menor para el relleno de poligonos //
				int j;
				Bordes temporal;
				for(int ii = 1; ii<cant; ii++){
					temporal = vertice_aux[ii]; //Se crea respaldo de ese valor
					j=ii-1;				// j aumenta conforme aumenta i
					while((temporal.y_max > vertice_aux[j].y_max)&& (j >= 0)){
						vertice_aux[j+1]=vertice_aux[j];
						j=j-1;
					}
					vertice_aux[j+1]= temporal; //Aqui se utiliza el respaldo
				}

				pintar_aux->bordes_ord = vertice_aux; // Guardamos el array ordenado de las coordenadas

				carro_pintado[part] = pintar_aux;


				//inicializamos valores para el siguiente poligono
				max_pol=-1;
				min_pol=2000;
				cant=0;
				vertice = espacio_partes(); // reseteamos para el siguiente poligono
				vertice_aux = espacio_partes();
				pintar_aux = espacio_pintado();
				part++;
			}
		}
	}
	bumper_t=carro[0];
	foco_t = carro[1];
	guarda_t= carro[2];
	cola=carro[3];
	carroceria=carro[4];
	llanta_t=carro[5];
	parab_t=carro[6];
	parab_d=carro[7];
	guarda_d=carro[8];
	llanta_d=carro[9];
	foco_d=carro[10];
	ventana = carro[11];
	div_w = carro[12];
	retro=carro[13];
	detalle1 = carro[14];
	detalle2 = carro[15];
	detalle3 = carro[16];
	detalle4 = carro[17];

	bumper_tp=carro_pintado[0];
	foco_tp = carro_pintado[1];
	guarda_tp= carro_pintado[2];
	colap=carro_pintado[3];
	carroceriap=carro_pintado[4];
	llanta_tp=carro_pintado[5];
	parab_tp=carro_pintado[6];
	parab_dp=carro_pintado[7];
	guarda_dp=carro_pintado[8];
	llanta_dp=carro_pintado[9];
	foco_dp=carro_pintado[10];
	ventana_p=carro_pintado[11];
	div_p = carro_pintado[12];
	retro_p = carro_pintado[13];
	detalle1_p = carro_pintado[14];
	detalle2_p = carro_pintado[15];
	detalle3_p = carro_pintado[16];
	detalle4_p = carro_pintado[17];
}


 //NO FUNCIONA //

// void rotar_imagen(){
// 	Bordes* carro[18] = {bumper_t, foco_t, guarda_t, cola, carroceria,llanta_t, parab_t,parab_d,guarda_d,llanta_d,foco_d, ventana, div_w, retro,detalle1,detalle2,detalle3,detalle4};
// 	pintar* carro_pintado[18] = {bumper_tp, foco_tp, guarda_tp, colap, carroceriap,llanta_tp, parab_tp,parab_dp,guarda_dp,llanta_dp,foco_dp,ventana_p,div_p,retro_p,detalle1_p,detalle2_p,detalle3_p,detalle4_p};

// 	//if(angulo != 0){
// 		int xi, yi, xf, yf;
// 		int xi_r, yi_r, xf_r, yf_r;
// 		int refx = (WIDTH/2) - 1;
// 		int refy = 356;
// 		double rad = 0.0;
// 		double rotacion = 0;

// 		if(angulo < 0 && angulo >= -360){
// 		      rotacion = 360 + angulo;
// 		}else if(angulo >= 0 && angulo <= 360){
// 		      rotacion = angulo;
// 		}
// 		rad = rotacion * (3.141592653589 / 180);

// 		for(int j = 0; j< 18; j++){
// 			int borde= 0; // indice para el borde del poligono
// 			int max_pol = -1; int min_pol = 2000;
// 			int cant = carro_pintado[j]->cantidad;
			
// 			while(borde < cant){
				
// 				//trasladamos un punto arbitrario al origen //
// 				xi = carro[j][borde].x0 - refx;
// 				yi = carro[j][borde].y0 - refy;
// 				xf = carro[j][borde].x1 - refx;
// 				yf = carro[j][borde].y1 - refy;

// 				// Rotamos //
// 				xi_r = (int)xi*cos(rad) - sin(rad)*yi;
// 				yi_r = (int)yi*cos(rad) - sin(rad)*xi;
// 				xf_r = (int)xf*cos(rad) - sin(rad)*yf;
// 				yf_r = (int)yf*cos(rad) - sin(rad)*xf;
		
// 				// Volvemos a posicion original //
// 				carro[j][borde].x0 = xi_r + refx;
// 				carro[j][borde].y0 = yi_r + refy;
// 				carro[j][borde].x1 = xf_r + refx;
// 				carro[j][borde].y1 = yf_r + refy;
				
// 				// Ahora se vuelve a escoger el y_max y el y_min //
// 				if(carro[j][borde].y0>carro[j][borde].y1){
// 					carro[j][borde].y_max = carro[j][borde].y0;
// 					carro[j][borde].y_min = carro[j][borde].y1; 

// 					//Aqui escogemos el maximo y el minimo en "y" para el poligono iterado 
// 					if(carro[j][borde].y0>max_pol)
// 						max_pol=carro[j][borde].y0;
					
// 					if(carro[j][borde].y1<min_pol){
// 						min_pol=carro[j][borde].y1;
// 					}
// 				}
// 				else if(carro[j][borde].y0<=carro[j][borde].y1){
// 					carro[j][borde].y_max = carro[j][borde].y1;
// 					carro[j][borde].y_min = carro[j][borde].y0;

// 					if(carro[j][borde].y1>max_pol)
// 						max_pol=carro[j][borde].y1;
					
// 					if(carro[j][borde].y0<min_pol){
// 						min_pol=carro[j][borde].y0;
// 					}
// 				}
// 				borde ++;	
// 			}

// 			// Ordenamos los bordes de mayor "y" a un "y" menor para el relleno de poligonos //
// 			Bordes* auxiliar = carro[j];
// 			int jj;
// 			Bordes temporal;
// 			for(int ii = 1; ii< cant; ii++){
// 				temporal = auxiliar[ii]; //Se crea respaldo de ese valor
// 				jj=ii-1;				// j aumenta conforme aumenta i
// 				while((temporal.y_max > auxiliar[jj].y_max)&& (jj >= 0)){
// 					auxiliar[jj+1]=auxiliar[jj];
// 					jj=jj-1;
// 				}
// 				auxiliar[jj+1]= temporal; //Aqui se utiliza el respaldo
// 			}

// 			carro_pintado[j]-> bordes_ord = auxiliar;
// 			carro_pintado[j]-> max_y = max_pol;
// 			carro_pintado[j]-> min_y = min_pol;
// 		}

// 		bumper_t=carro[0];
// 		foco_t = carro[1];
// 		guarda_t= carro[2];
// 		cola=carro[3];
// 		carroceria=carro[4];
// 		llanta_t=carro[5];
// 		parab_t=carro[6];
// 		parab_d=carro[7];
// 		guarda_d=carro[8];
// 		llanta_d=carro[9];
// 		foco_d=carro[10];
// 		ventana = carro[11];
// 		div_w = carro[12];
// 		retro=carro[13];
// 		detalle1 = carro[14];
// 		detalle2 = carro[15];
// 		detalle3 = carro[16];
// 		detalle4 = carro[17];

// 		bumper_tp=carro_pintado[0];
// 		foco_tp = carro_pintado[1];
// 		guarda_tp= carro_pintado[2];
// 		colap=carro_pintado[3];
// 		carroceriap=carro_pintado[4];
// 		llanta_tp=carro_pintado[5];
// 		parab_tp=carro_pintado[6];
// 		parab_dp=carro_pintado[7];
// 		guarda_dp=carro_pintado[8];
// 		llanta_dp=carro_pintado[9];
// 		foco_dp=carro_pintado[10];
// 		ventana_p=carro_pintado[11];
// 		div_p = carro_pintado[12];
// 		retro_p = carro_pintado[13];
// 		detalle1_p = carro_pintado[14];
// 		detalle2_p = carro_pintado[15];
// 		detalle3_p = carro_pintado[16];
// 		detalle4_p = carro_pintado[17];

// 	//}

// }

void only_b(){ //Funcion a utilizar para pintar unicamente los bordes
	Bordes *carro[18] = {bumper_t,foco_t,guarda_t,cola,carroceria,llanta_t,parab_t,parab_d,guarda_d,llanta_d,foco_d,ventana,div_w,retro,detalle1,detalle2,detalle3,detalle4};
	for(int part = 0; part<18; part++){
		int i=0;
		while(carro[part][i].x0 !=0 && carro[part][i].y0 !=0 && carro[part][i].x1 !=0 && carro[part][i].y1 !=0){
			bresenham(carro[part][i].x0,carro[part][i].y0,carro[part][i].x1,carro[part][i].y1);
			i++;
		}
	}
}
	
void activar_bordes(int cant_bordes, pintar* parteApintar, int max_scanline){
	for(int i = 0; i<cant_bordes; i++){
	
		if(parteApintar->bordes_ord[i].activo == 0){ // solo para activar los bordes que no estan activos
			
			if(parteApintar->bordes_ord[i].y_max == max_scanline){

				double x0=parteApintar->bordes_ord[i].x0;
				double y0=parteApintar->bordes_ord[i].y0;
				double x1=parteApintar->bordes_ord[i].x1;
				double y1=parteApintar->bordes_ord[i].y1;
				if(x0 != x1){
					parteApintar->bordes_ord[i].m = ((y1-y0)/(x1-x0)); //Problema porque redondea cuando no queremos
				}
				else {
					parteApintar->bordes_ord[i].m = 0;
				}
				
				
				if(y0 != y1){
					parteApintar->bordes_ord[i].delta_m = (x1-x0)/(y0-y1); // calculamos el delta para la siguiente interseccion
				}
				else {
					parteApintar->bordes_ord[i].delta_m = 0;
				}

				
				if((parteApintar->bordes_ord[i].y_max == parteApintar->bordes_ord[i].y0 )){
					parteApintar->bordes_ord[i].x_anterior=x0;
				}
				else if (parteApintar->bordes_ord[i].y_max == parteApintar->bordes_ord[i].y1 ){
					parteApintar->bordes_ord[i].x_anterior=x1;
				}
				parteApintar->bordes_ord[i].activo = 1; //activamos el borde
				//printf("coord: (%f, %f)-(%f, %f) m: %f\n", x0,y0,x1,y1,parteApintar->bordes_ord[i].m);
				bordes_activos++;
			}
		}
	}

}

int activos_Y_min(pintar* parteApintar, int cant_bordes){ // Busca entre los bordes activos el mayor valor de los min_y
	int aux = -1;
	for(int i=0; i<cant_bordes; i++){

		if (parteApintar->bordes_ord[i].activo == 1){ // solo verifica con los bordes activos
			if(parteApintar->bordes_ord[i].y_min > aux){
				aux=parteApintar->bordes_ord[i].y_min;
			}
		}
	}
	
	return aux;
}

void borrar_activo(int minimoY_activos, pintar* parteApintar, int cant_bordes){
	for(int i = 0; i<cant_bordes; i++){
		if(parteApintar->bordes_ord[i].y_min == minimoY_activos){
			// printf("desact = %i  min: (%i) max: %i \n", i,parteApintar->bordes_ord[i].y_min,parteApintar->bordes_ord[i].y_max );
			parteApintar->bordes_ord[i].activo = 0;
			bordes_activos --;
		}
	}
}

void calc_intersecciones(){  //Funcion donde se calculan las intersecciones de los bordes con el scanline


	pintar* carro_pintado[18] = {bumper_tp, foco_tp, guarda_tp, colap, carroceriap,llanta_tp, parab_tp,parab_dp,guarda_dp,llanta_dp,foco_dp,ventana_p, div_p,retro_p,detalle1_p,detalle2_p,detalle3_p,detalle4_p};
	intersec_partes *intersec_pintado[18] ={bumper_ti, foco_ti, guarda_ti, colai, carroceriai,llanta_ti, parab_ti,parab_di,guarda_di,llanta_di,foco_di,ventana_pi,div_pi,retro_pi,detalle1_pi,detalle2_pi,detalle3_pi,detalle4_pi};

	for(int i = 0; i<18; i ++){
		pintar *parteApintar= carro_pintado[i];
		int max_y=parteApintar->max_y; 		//maximo del poligono a pintar
		int min_y=parteApintar->min_y;
		int cant_bordes = parteApintar->cantidad; //cantidad de bordes del poligono
		int real_tam = max_y - min_y + 1; //tamano total en y en espacios de memoria de ese poligono
		intersec_partes *inter_detect = (intersec_partes *)malloc(real_tam * sizeof(intersec_partes)); //Para guardar las intersecciones detectadas de cada scanline
		int iterado = 0; 		// Recorrer la estructura "inter_detect"
		bordes_activos = 0;

		while(max_y >= min_y){
			
			activar_bordes(cant_bordes, parteApintar, max_y); //se activa bordes de inicio de ese poligono
			int minimoY_activos = 0;  // Valor del minimo "y" de lo bordes activos
			
			//printf("%i\n", bordes_activos);

			if(bordes_activos >= 2){

				int scanline = max_y;				
				minimoY_activos = activos_Y_min(parteApintar, cant_bordes);

				/*aqui movemos el scanline hasta que lleguemos al 'y' de minimo valor
				  asi desactivamos el borde al que ya se llego a su 'y' min */
				while(scanline >= minimoY_activos){ 
					
					activar_bordes(cant_bordes, parteApintar, scanline); //Activamos bordes segun el scanline
					interseccion *inter_aux = (interseccion *)malloc(1000*sizeof(interseccion)); //Aqui guardamos las intersecciones para cada scanline
					int i_intersec = 0;
					minimoY_activos = activos_Y_min(parteApintar, cant_bordes);

					/*se calcula la interseccion de cada borde activo del poligono*/
					for(int j=0; j<cant_bordes; j++){
						if(parteApintar->bordes_ord[j].activo == 1){ //Solo calcula interseccion para los bordes activos
							int x=0;
							double m_borde = parteApintar->bordes_ord[j].m;
							if(m_borde != 0){
								x = (int)(parteApintar->bordes_ord[j].x_anterior - (1/m_borde));	
							}
							else 
								x = parteApintar->bordes_ord[j].x_anterior;

							if(scanline != parteApintar->bordes_ord[j].y_min){
								inter_aux[i_intersec].int_x = x;
								inter_aux[i_intersec].int_y = scanline;
								inter_aux[i_intersec].min_b = parteApintar->bordes_ord[j].y_min;
								inter_aux[i_intersec].max_b = parteApintar->bordes_ord[j].y_max;
								parteApintar->bordes_ord[j].x_anterior=x;
								//printf("coordenadas (%i, %i) min: %i  max: %i  m: %f\n", x,scanline,inter_aux[i_intersec].min_b,inter_aux[i_intersec].max_b,m_borde);
								i_intersec ++;
								
							}
						}
					}

					/*Ahora se ordenan las intersecciones para ese scanline segun el valor de 'x', de menor a mayor*/
					int jj;
					interseccion temp_inter;
					for(int ii = 1; ii<i_intersec; ii ++){
						temp_inter = inter_aux[ii];
						jj= ii-1;
						while((inter_aux[jj].int_x > temp_inter.int_x) && (jj>=0)){
							inter_aux[jj+1] = inter_aux[jj];
							jj=jj-1;
						}
						inter_aux[jj+1] = temp_inter;
					}

					/* Ahora se eliminan las dobles intersecciones que corresponden a un vertice del poligolo
					   las cuales en lugar de 2 se deben interpretar como 1 para el caso donde tenemos vertices
					   de la siguiente forma ( < 0 > )*/

					interseccion* inter_aux_dep = (interseccion *)malloc(i_intersec*sizeof(interseccion));
					int i_dep=0;
					int index=0;
					while(index <i_intersec){
						if(index+1 < i_intersec){
							/*Se comprueba con el max y el minimo de cada borde con respecto al siguiente*/
							// Si son iguales guardamos uno y brincamos al segundo borde siguiente al que se guarda //
							if(inter_aux[index].min_b == inter_aux[index+1].max_b || inter_aux[index].max_b == inter_aux[index+1].min_b){
								inter_aux_dep[i_dep] = inter_aux[index];
								index ++;
								i_dep ++;
							}
							else{ // Si no son iguales guardamos y pasamos al siguiente borde
								inter_aux_dep[i_dep] = inter_aux[index];
								i_dep ++;
							}
						}
						else{
							inter_aux_dep[i_dep] = inter_aux[index];
							i_dep ++;
						}

						//printf("inter: (%i, %i) max: %i  min: %i \n",inter_aux_dep[i_dep].int_x,inter_aux_dep[i_dep].int_y,inter_aux_dep[i_dep].max_b,inter_aux_dep[i_dep].min_b );
						index ++;
					}

					free(inter_aux); //eliminamos los parametros correspondientes a este auxiliar

					/* Guardamos las intersecciones correspondientes a ese scanline */
					inter_detect[iterado].intersecciones = inter_aux_dep;
					inter_detect[iterado].cant = i_dep;
					iterado ++;

					scanline --; //Pasamos al siguiente scanline

				}

				max_y =  scanline+1; // ahora el max_y lo ubicamos donde queda el scanline
				if(max_y == min_y) //se ha terminado con este poligono
					break;  		 
				else 			   //Eliminamos borde completado
					borrar_activo(minimoY_activos, parteApintar, cant_bordes);
			}
			max_y--;
		}
		intersec_pintado[i] = inter_detect;

	}

	bumper_ti = intersec_pintado[0];
	foco_ti= intersec_pintado[1];
	guarda_ti= intersec_pintado[2];
	colai= intersec_pintado[3];
	carroceriai= intersec_pintado[4];
	llanta_ti= intersec_pintado[5];
	parab_ti= intersec_pintado[6];
	parab_di= intersec_pintado[7];
	guarda_di= intersec_pintado[8];
	llanta_di= intersec_pintado[9];
	foco_di= intersec_pintado[10];
	ventana_pi = intersec_pintado[11];
	div_pi = intersec_pintado[12];
	retro_pi = intersec_pintado[13];
	detalle1_pi = intersec_pintado[14];
	detalle2_pi = intersec_pintado[15];
	detalle3_pi = intersec_pintado[16];
	detalle4_pi = intersec_pintado[17];
}

void scanline(int parteApintar, int r, int g, int b){ //Funcion encargadar de pintar las partes del carro
	pintar* carro_pintado[18] = {bumper_tp, foco_tp, guarda_tp, colap, carroceriap,llanta_tp, parab_tp,parab_dp,guarda_dp,llanta_dp,foco_dp,ventana_p, div_p,retro_p,detalle1_p,detalle2_p,detalle3_p,detalle4_p};
	intersec_partes *intersec_pintado[18] ={bumper_ti, foco_ti, guarda_ti, colai, carroceriai,llanta_ti, parab_ti,parab_di,guarda_di,llanta_di,foco_di,ventana_pi,div_pi,retro_pi,detalle1_pi,detalle2_pi,detalle3_pi,detalle4_pi};

	int max_y = carro_pintado[parteApintar]->max_y; //maximo en 'y' de ese poligono
	int min_y = carro_pintado[parteApintar]->min_y; //Minimo en 'y' de ese poligono
	int tam = max_y - min_y;   // Tamano en y del poligono

	intersec_partes *aux= intersec_pintado[parteApintar];
	SDL_SetRenderDrawColor(app.render, r, g, b, 0);

	for(int i = 0; i<tam; i++){
		int cant_intersec = aux[i].cant;
		int print = 1;    // Para pintar de manera intercalada
		for(int j = 0; j<cant_intersec; j++){
			if(print == 1 && j+1 < cant_intersec){
				int x0 = aux[i].intersecciones[j].int_x;
				int y0 = aux[i].intersecciones[j].int_y;
				int x1 = aux[i].intersecciones[j+1].int_x;
				int y1 = aux[i].intersecciones[j+1].int_y;
				//printf("(%i, %i) hasta (%i, %i)\n", x0,y0,x1,y1);
				bresenham(x0,y0,x1,y1);
				print=0;
			}
			else{
				print=1;
			}
		} 
	}
}

void scanline_texture(int parteApintar, int textura){
	leer_imagen(textura);

	pintar* carro_pintado[18] = {bumper_tp, foco_tp, guarda_tp, colap, carroceriap,llanta_tp, parab_tp,parab_dp,guarda_dp,llanta_dp,foco_dp,ventana_p, div_p,retro_p,detalle1_p,detalle2_p,detalle3_p,detalle4_p};
	intersec_partes *intersec_pintado[18] ={bumper_ti, foco_ti, guarda_ti, colai, carroceriai,llanta_ti, parab_ti,parab_di,guarda_di,llanta_di,foco_di,ventana_pi,div_pi,retro_pi,detalle1_pi,detalle2_pi,detalle3_pi,detalle4_pi};

	int max_y = carro_pintado[parteApintar]->max_y; //maximo en 'y' de ese poligono
	int min_y = carro_pintado[parteApintar]->min_y; //Minimo en 'y' de ese poligono
	int tam = max_y - min_y;   // Tamano en y del poligono

	intersec_partes *aux= intersec_pintado[parteApintar];

	for(int i = 0; i<tam; i++){
		int cant_intersec = aux[i].cant;
		int print = 1;    // Para pintar de manera intercalada
		for(int j = 0; j<cant_intersec; j++){
			if(print == 1 && j+1 < cant_intersec){
				int x0 = aux[i].intersecciones[j].int_x;
				int y0 = aux[i].intersecciones[j].int_y;
				int x1 = aux[i].intersecciones[j+1].int_x;
				int y1 = aux[i].intersecciones[j+1].int_y;
				//printf("(%i, %i) hasta (%i, %i)\n", x0,y0,x1,y1);
				bresenham(x0,y0,x1,y1);
				print=0;
			}
			else{
				print=1;
			}
		} 
	}
	free(R);
	free(G);
	free(B);
}

void zoom_pan(){
	if(app.operacion != 0){
		if(app.operacion == 1){ //zoom acercar
			clean_window();
			if(escalx_g < 1){
	            escalx_g+=0.1;
	            escaly_g+=0.1;
	        }else if(escalx_g <= 2.5){
	            escalx_g+=0.1;
	            escaly_g+=0.1;
	        }
		}

		if(app.operacion == 2){ // Zoom alejar
			clean_window();
			if(escalx_g > 0.1){
	            escalx_g-=0.1;
	            escaly_g-=0.1;
	        }
		}

		if(app.operacion == 4){ //pan hacia abajo
			clean_window();
	        if(pany_g + 20 <= escaly_g * 500){
	            pany_g += 20;
	        }
	    }
	    if(app.operacion == 3){ //pan hacia arriba
	    	clean_window();
	        if(pany_g - 20 >= escaly_g * -500){
	            pany_g -= 20;
	        }
	    }
	    /// Quede aqui

	    if(app.operacion == 6){ //pan hacia derecha
	    	clean_window();
	        if(panx_g - 20 >= escalx_g * -720){
	            panx_g -= 20;
	        }
	    }
	    if(app.operacion == 5){ //pan hacia izquierda
	    	clean_window();
	        if(pany_g + 20 <= escalx_g * 720){
	            panx_g += 20;
	        }
	    }

	    if(app.operacion == 7){ // Para resetear las operaciones
	    	clean_window();
	    	panx_g = 0;
		    pany_g = 0;
		    escalx_g = 1;
		    escaly_g = 1;
		    angulo = 0;
	    }

	    // if(app.operacion == 8){ // Para rotar a la izquierda
	    // 	clean_window();
	    // 	if(angulo-15 > -360){
     //        	angulo -= 15;
	    //     }else if(angulo == -360){
	    //         angulo = 0;
	    //     }

	    // }
	    // if(app.operacion == 9){
	    // 	clean_window();
	    // 	if(angulo+15 < 360){
     //        	angulo += 15;
	    //     }else if(angulo == 360){
	    //         angulo = 0;
	    //     }
	    // }

	}
}

/////////////  Para el manejo de texturas ////////////////////////
void leer_imagen(int imagen){
	FILE *archivo; //Permite manipular el archivo de la imagen

	if(imagen == 1){
		archivo = fopen("car2.ppm","r");
	}
	
	if(imagen == 2){
		archivo = fopen("carroceria.ppm","r");
	}
	if(imagen == 3){
		archivo = fopen("llanta.ppm","r");
	}
	if(imagen == 4){
		archivo = fopen("guardab.ppm","r");
	}
	if(imagen == 5){
		archivo = fopen("detalles.ppm","r");
	}
	if(imagen == 6){
		archivo = fopen("ventanas.ppm","r");
	}
	if(imagen == 7){
		archivo = fopen("parabrisas.ppm","r");
	}
	if(imagen == 8){
		archivo = fopen("focot.ppm","r");
	}
	if(imagen == 9){
		archivo = fopen("detalle1.ppm","r");
	}
	if(imagen == 10){
		archivo = fopen("focod.ppm","r");
	}
	if(imagen == 11){
		archivo = fopen("espejos.ppm","r");
	}


	// Ahora leemos el encabezado del archivo //
	char letra;
	int tipo, maxcolor;
	fscanf(archivo, "%c%d",&letra,&tipo); //lee una letra-tipo de imagen-vaalor ancho-valor alto-maximo color de cada pixel
	fscanf(archivo, "%d %d", &TW, &TH);
	fscanf(archivo, "%d", &maxcolor);

	//printf("Tipo: %c%i\nAncho y alto: %i %i\ncolor max: %i\n", letra, tipo, TW,TH,maxcolor);
	
	R=(int **)malloc(TH*sizeof(int *));
	for(int i = 0; i<TH; i++){
		R[i]= (int *)malloc(TW*sizeof(int));
	}

	G=(int **)malloc(TH*sizeof(int *));
	for(int i = 0; i<TH; i++){
		G[i]= (int *)malloc(TW*sizeof(int));
	}
	B=(int **)malloc(TH*sizeof(int *));
	for(int i = 0; i<TH; i++){
		B[i]= (int *)malloc(TW*sizeof(int));
	}

	/* Aqui se extraen los datos del archivo y se guardan*/
	for(int i = 0; i<TH; i++){
		for(int j=0; j<TW; j++){
			fscanf(archivo, "%d %d %d  ", &R[i][j],&G[i][j],&B[i][j]);
			
		}
	}
	fclose(archivo);
}

void set_color_texture(int x, int scanline, int TH_I, int TW_I){
	int red, green, blue;

	red = R[scanline % TH_I][x % TW_I];
	green = G[scanline % TH_I][x % TW_I];
	blue = B[scanline % TH_I][x % TW_I];

	SDL_SetRenderDrawColor(app.render, red, green, blue, 0);
	//printf("%i %i %i", red,green,blue);
}

void dibujar(){

	if(app.modo != aux_t){
		aux_t = app.modo;
		clean_window();
	}

	coord = fopen("coordenadas.txt", "r");
	if(coord == NULL)
		printf("Error al abrir el archivo\n");

	vertices_polig(escaly_g,escalx_g, pany_g, panx_g);

	fclose(coord);
	
	//rotar_imagen();

	if(app.modo==1){ // Solo bordes

		SDL_SetRenderDrawColor(app.render, 10, 10, 10, 0);
		only_b();
	}

	if(app.modo==2){ // Poligonos rellenos
		calc_intersecciones();
		scanline(0, 255,255,255); //Para pintar el bumper trasero
		scanline(1,255,0,0); //Para pintar el foco trasero
		scanline(2,50,50,50); //Para pintar el guardabarro trasero
		scanline(3,50,50,50); //Para pintar cola
		scanline(4, 255,255,255); //Para pintar carroceria
		scanline(5,15,15,15); //Para pintar llanta trasera
		scanline(6,132,198,237); //Para pintar parabrisas trasero
		scanline(7,132,198,237); //Para pintar parabrisas delantero
		scanline(8,50,50,50); //Para pintar el guardabarro delantero
		scanline(9,15,15,15); //Para pintar llanta trasera
		scanline(10,221,221,221); //Para pintar el foco delantero
		scanline(11,0,0,0); //Para pintar la ventana
		scanline(12,50,50,50); // Para pintar division de ventana polarizada 
		scanline(13,25,25,25); //Para pintar espejos
		scanline(14,25,25,25); // detalle1 carroceria
		scanline(15,25,25,25); // detalle2 guardabarro
		scanline(16,255,128,0); // detalle3 foco delantero
		scanline(17,25,25,25); // detalle1 carroceria
	}

	if(app.modo==3){  //Relleno con textura

		calc_intersecciones();
		scanline_texture(0,1); //Para pintar el bumper trasero
		scanline_texture(1,1); //Para pintar el foco trasero
		scanline_texture(2,1); //Para pintar el guardabarro trasero
		scanline_texture(3,1); //Para pintar cola
		scanline_texture(4,1); //Para pintar carroceria
		scanline_texture(5,1); //Para pintar llanta trasera
		scanline_texture(6,1); //Para pintar parabrisas delantero
		scanline_texture(8,1); //Para pintar el guardabarro delantero
		scanline_texture(9,1); //Para pintar parabrisas trasero
		scanline_texture(7,1); //Para pintar llanta trasera
		scanline_texture(10,1); //Para pintar el foco delantero
		scanline_texture(11,1); //Para pintar la ventana
		scanline_texture(12,1); // Para pintar division de ventana polarizada 
		scanline_texture(13,1); //Para pintar espejos
		scanline_texture(14,1); // detalle1 carroceria
		scanline_texture(15,1); // detalle2 guardabarro
		scanline_texture(16,1); // detalle3 foco delantero
		scanline_texture(17,1); // detalle1 carroceria
	}

	if(app.modo == 4){
		calc_intersecciones();
		scanline_texture(0,2); //Para pintar el bumper trasero
		scanline_texture(1,8); //Para pintar el foco trasero
		scanline_texture(2,4); //Para pintar el guardabarro trasero
		scanline_texture(3,4); //Para pintar cola
		scanline_texture(4,2); //Para pintar carroceria
		scanline_texture(5,3); //Para pintar llanta trasera
		scanline_texture(6,7); //Para pintar parabrisas trasero
		scanline_texture(7,7); //Para pintar parabrisas delantero
		scanline_texture(8,4); //Para pintar el guardabarro delantero
		scanline_texture(9,3); //Para pintar llanta trasera
		scanline_texture(10,10); //Para pintar el foco delantero
		scanline_texture(11,6); //Para pintar la ventana
		scanline_texture(12,11); // Para pintar division de ventana polarizada 
		scanline_texture(13,11); //Para pintar espejos
		scanline_texture(14,5); // detalle1 carroceria
		scanline_texture(15,4); // detalle2 guardabarro
		scanline_texture(16,9); // detalle3 foco delantero
		scanline_texture(17,5); // detalle1 carroceria
	}
	if(app.firma == 1){
		leer_firma();

	}

	if(app.firma == 2){
		if(puntos_bezier == 4){
			curvas_Bezier(X,Y);
			puntos_bezier = 0;
		}
	}
}

void leer_firma(){

	FILE *firma;
	
	int x[4], y[4];
	firma = fopen("firma.txt", "r");
	rewind(firma);
	while(!feof(firma)){
		for(int i = 0; i<4; i++){
			fscanf(firma,"%d,%d", &x[i], &y[i]);
		}
		curvas_Bezier(x,y);
	}
	fclose(firma);	
}

void crear_espacio(){ //Para reservar espacio en memoria de las matrices de puntos de Bezier
	X=(int *)malloc(4*sizeof(int));
	Y=(int *)malloc(4*sizeof(int));
}

void curvas_Bezier(int x[], int y[]){
	double Bx = 0.0, By = 0.0;

	for(double t = 0.0 ; t<= 1.0; t += 0.0001){
		Bx=pow(1-t,3)*x[0] + 3*t*pow(1-t,2)*x[1] + 3*pow(t,2)*(1-t)*x[2] + pow(t,3)*x[3];
		By=pow(1-t,3)*y[0] + 3*t*pow(1-t,2)*y[1] + 3*pow(t,2)*(1-t)*y[2] + pow(t,3)*y[3];
		SDL_SetRenderDrawColor(app.render, 0,0,0,0);
		SDL_RenderDrawPoint(app.render,(int)Bx, (int)By);		
	}
}

int main(){
	// Valores por defecto de las operaciones
	panx_g = 0;
    pany_g = 0;
    escalx_g = 1;
    escaly_g = 1;
    app.operacion = 0;
    angulo = 0;

	init_SDL();
	clean_window();
	crear_espacio();
	while (1) {

        input();
        zoom_pan();
        dibujar();
        SDL_Delay(4); // Para esperar 4 ms
        SDL_RenderPresent(app.render);
    }

    SDL_DestroyWindow(app.window);
    SDL_DestroyRenderer(app.render);
    SDL_Quit();
    free(X);
    free(Y);

	return 0;
}