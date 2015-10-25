// *******************************************************
// CS 174a Graphics Example Code
// Animation.cpp - The main file and program start point.  The class definition here describes how to display an Animation and how it will react to key and mouse input.  Right now it has 
// very little in it - you will fill it in with all your shape drawing calls and any extra key / mouse controls.  

// Now go down to display() to see where the sample shapes are drawn, and to see where to fill in your own code.

#include "../CS174a template/Utilities.h"
#include "../CS174a template/Shapes.h"
#include "../CS174a template/GLUT_Port.h"
#include <stack>
int g_width = 800, g_height = 800 ;


// *******************************************************	
// When main() is called it creates an Animation object, which registers itself as a displayable object to our other class GL_Context -- which OpenGL is told to call upon every time a
// draw / keyboard / mouse event happens.
class Animation : public DisplayObject
{
	KeyMap<Animation> keys;
	GLUT_Port g;
	Matrix4d projection_transform, camera_transform;		
	Vector3d thrust;
	Triangle_Fan_Full* m_fan;
	Rectangular_Strip* m_strip;
	Text_Line* m_text;
	Cylindrical_Strip* m_tube;
	Cube* m_cube;
	Sphere* m_sphere;
	Axis* axis;
	
	bool looking, animate, gouraud, color_normals, solid;
	float animation_time;

public:
	// projection_transform:  The matrix that determines how depth is treated.  It projects 3D points onto a plane.
	Animation() : projection_transform( Perspective( 45, float(g_width)/g_height, .1f, 100 ) ),	
		camera_transform( Matrix4d::Identity() ),		g( Vector2i(800, 20), Vector2i(600, 600), "CS174a Template Code"), looking(0), animation_time(0), animate(0), 
		gouraud(0), color_normals(0), solid(0), thrust( Vector3d::Zero() )
	{			
		g.registerDisplayObject(*this);
		g.init( "vshader.glsl", "fshader.glsl" );
			
		glClearColor( .8f, .8f, .9f, 1 );								// Background color

		m_fan = new Triangle_Fan_Full( Matrix4d::Identity() );
		m_strip = new Rectangular_Strip( 1, Matrix4d::Identity() );
		m_text = new Text_Line( 6, Matrix4d::Identity() );
		m_text->set_string( "normal" );
		m_tube = new Cylindrical_Strip( 10, Matrix4d::Identity() );
		m_cube = new Cube( Matrix4d::Identity() );
		m_sphere = new Sphere( Matrix4d::Identity(), 3 );
		axis = new Axis( Matrix4d::Identity() );
	}
	
	void update_camera( float animation_delta_time, Vector2d &window_steering )
	{
		const unsigned leeway = 70, border = 50;
		float degrees_per_frame = .02f * animation_delta_time;
		float meters_per_frame  = 10.f * animation_delta_time;
																									// Determine camera rotation movement first
		Vector2f movement_plus  ( window_steering[0] + leeway, window_steering[1] + leeway );		// movement[] is mouse position relative to canvas center; leeway is a tolerance from the center.
		Vector2f movement_minus ( window_steering[0] - leeway, window_steering[1] - leeway );
		bool outside_border = false;
	
		for( int i = 0; i < 2; i++ )
			if ( abs( window_steering[i] ) > g.get_window_size()[i]/2 - border )	outside_border = true;		// Stop steering if we're on the outer edge of the canvas.

		for( int i = 0; looking && outside_border == false && i < 2; i++ )			// Steer according to "movement" vector, but don't start increasing until outside a leeway window from the center.
		{
			float angular_velocity = ( ( movement_minus[i] > 0) * movement_minus[i] + ( movement_plus[i] < 0 ) * movement_plus[i] ) * degrees_per_frame;	// Use movement's quantity conditionally
			camera_transform = Affine3d( AngleAxisd( angular_velocity, Vector3d( i, 1-i, 0 ) ) ) * camera_transform;			// On X step, rotate around Y axis, and vice versa.			
		}
		camera_transform = Affine3d(Translation3d( meters_per_frame * thrust )) * camera_transform;		// Now translation movement of camera, applied in local camera coordinate frame
	}
	
	// *******************************************************	
	// display(): called once per frame, whenever OpenGL decides it's time to redraw.
	virtual void display(float animation_delta_time, Vector2d &window_steering)
	{		
		if( animate ) animation_time += animation_delta_time;

		update_camera( animation_delta_time , window_steering );

		int basis_id = 0;

		Matrix4d model_transform = Matrix4d::Identity();
		
		/**********************************
		Start coding here!!!!
		**********************************/

		Matrix4d s[10];

		model_transform *= Affine3d(Translation3d(0, -7, -30)).matrix();
		model_transform *= Affine3d(AngleAxisd(PI / 6, Vector3d(0, 1,0 ))).matrix();		//  Rotate

		s[0] = model_transform; // ground level center

			//draw ground
			glUniform4fv( g_addrs->color_loc, 			1, Vector4f( .3f,.8f,.4f,1 ).data() );	// Send a desired shape color to the graphics card
			model_transform *= Affine3d(AngleAxisd(  PI/2 , Vector3d(0,0, -1))).matrix();		//  Rotate
			axis->draw(basis_id++, projection_transform, camera_transform, model_transform, "");
			model_transform *= Vector4d(20, 20, 20, 1).asDiagonal();							//  Scale
			m_strip->draw(projection_transform, camera_transform, model_transform, "");			// Rectangle ground
		
		model_transform = s[0];//back to ground center

			//draw trunck
			glUniform4fv(g_addrs->color_loc, 1, Vector4f(.5f, .2f, .2f, 1).data());	// Send a desired shape color to the graphics card

			s[9] = model_transform;//save temporary position 

				//scaled cube
				model_transform *= Vector4d(0.25, 2, .25, 1).asDiagonal();
				m_cube->draw(projection_transform, camera_transform, model_transform, "");

			model_transform = s[9];

			for (int i = 0; i < 8; i++)
			{
				model_transform *= Affine3d(Translation3d(0, 1, 0)).matrix();//to the top of the previous box 
				model_transform *= Affine3d(AngleAxisd((sin(animation_time * 3) / 20.0), Vector3d(1, 0, 0))).matrix();		//rotate on the intersection
				model_transform *= Affine3d(Translation3d(0, 1, 0)).matrix();// go to the center of next box where it should be.
				
				s[9] = model_transform;//save temporary position 
					
					//scaled cube drawing
					model_transform *= Vector4d(0.25, 2, .25, 1).asDiagonal();
					m_cube->draw(projection_transform, camera_transform, model_transform, "");

				model_transform = s[9];// back to unscaled base

				axis->draw(basis_id++, projection_transform, camera_transform, model_transform, "");
			}
			model_transform *= Affine3d(Translation3d(0, 2, 0)).matrix();// goto the center of foliage, 1 unit above the top of last box

			s[1] = model_transform; // center of foliage
			
				//draw foliage
				glUniform4fv(g_addrs->color_loc, 1, Vector4f(.9f, .7f, .7f, 1).data());
				axis->draw(basis_id++, projection_transform, camera_transform, model_transform, "");
				model_transform *= Vector4d(2, 2, 2, 1).asDiagonal();
				m_sphere->draw(projection_transform, camera_transform, model_transform, "", 3);
				
		
		model_transform = s[0];//back to ground center

			//hinge for bee up and down + rotate
			model_transform *= Affine3d(AngleAxisd(animation_time * 50 * PI / 180, Vector3d(0, 1, 0))).matrix();
			model_transform *= Affine3d(Translation3d(6, 0, 0)).matrix();
			model_transform *= Affine3d(Translation3d(0, 5 + 0.6* sin(animation_time*5), 0)).matrix();

			s[2] = model_transform;// center of bee body

				//draw bee body
				glUniform4fv(g_addrs->color_loc, 1, Vector4f(.2f, .8f, .6f, 1).data());
				s[7] = model_transform;
					model_transform *= Affine3d(Translation3d(0, 0, .3)).matrix();
					model_transform *= Vector4d(.5, .5, 1.5, 1).asDiagonal();
					axis->draw(basis_id++, projection_transform, camera_transform, model_transform, "");
					m_sphere->draw(projection_transform, camera_transform, model_transform, "", 3);
				model_transform = s[7];
				//bee head
				glUniform4fv(g_addrs->color_loc, 1, Vector4f(.8f, .8f, .1f, 1).data());
					model_transform *= Affine3d(Translation3d(0, 0, -1.4)).matrix();
					model_transform *= Vector4d(.7, 0.5, 0.5, 1).asDiagonal();
					axis->draw(basis_id++, projection_transform, camera_transform, model_transform, "");
					m_sphere->draw(projection_transform, camera_transform, model_transform, "", 3);
				model_transform = s[7];
				//bee midpart
				glUniform4fv(g_addrs->color_loc, 1, Vector4f(.2f, .8f, .1f, 1).data());
					model_transform *= Affine3d(Translation3d(0, 0, -.7)).matrix();
					model_transform *= Vector4d(.4, .4, 0.5, 1).asDiagonal();
					axis->draw(basis_id++, projection_transform, camera_transform, model_transform, "");
					m_sphere->draw(projection_transform, camera_transform, model_transform, "", 3);
				model_transform = s[7];


			model_transform = s[2];//back to center of body

				// wings hinge
				model_transform *= Affine3d(Translation3d(0, .5, 0)).matrix();

				glUniform4fv(g_addrs->color_loc, 1, Vector4f(.9f, .7f, .9f, 1).data());

				s[3] = model_transform;// wings hinge

					// one wing
					model_transform *= Affine3d(AngleAxisd(5 - 0.6* sin(animation_time * 20), Vector3d(0, 0, 1))).matrix();
					model_transform *= Affine3d(Translation3d(0, 1, 0)).matrix();
					model_transform *= Affine3d(AngleAxisd(PI / 2, Vector3d(1, 0, 0))).matrix();
					model_transform *= Vector4d(1, 2, 2, 1).asDiagonal();
					m_strip->draw(projection_transform, camera_transform, model_transform, "");
					axis->draw(basis_id++, projection_transform, camera_transform, model_transform, "");
					
				model_transform = s[3];

					// other wing
					model_transform *= Affine3d(AngleAxisd(-5 + 0.6* sin(animation_time * 20), Vector3d(0, 0, 1))).matrix();
					model_transform *= Affine3d(Translation3d(0, 1, 0)).matrix();
					model_transform *= Affine3d(AngleAxisd(PI / 2, Vector3d(1, 0, 0))).matrix();
					model_transform *= Vector4d(1, 2, 2, 1).asDiagonal();
					m_strip->draw(projection_transform, camera_transform, model_transform, "");
					axis->draw(basis_id++, projection_transform, camera_transform, model_transform, "");
	

			model_transform = s[2]; //back to center of body

			    // leg hinges center 
				model_transform *= Affine3d(Translation3d(0, -.40, 0)).matrix();
				s[4] = model_transform;// leg hinges(2) center
					
					//right leg first level hinge
					model_transform *= Affine3d(Translation3d(.2, 0, 0)).matrix();
					s[5] = model_transform; //

						//draw legs
						glUniform4fv(g_addrs->color_loc, 1, Vector4f(.2f, .1f, .1f, 1).data());
						for (int i = 0; i < 3; i++)
						{

							s[9] = model_transform;

								//first level leg(hinge for first level leg)
								model_transform *= Affine3d(AngleAxisd(  PI/3 - 0.4* sin(animation_time * 5), Vector3d(0, 0, 1))).matrix();
								model_transform *= Affine3d(Translation3d(0, -.4/2, (i-1)/2.0)).matrix();
								s[8] = model_transform;// base point of first level legs for each

									model_transform *= Vector4d(0.2, .4, .2, 1).asDiagonal();
									m_cube->draw(projection_transform, camera_transform, model_transform, "");
									axis->draw(basis_id++, projection_transform, camera_transform, model_transform, "");
									
								model_transform = s[8];

								//second level leg(hinge point for second level legs)
								model_transform *= Affine3d(Translation3d(0, -.4/2, 0)).matrix();
								model_transform *= Affine3d(AngleAxisd(-PI / 3 - 0.4* sin(animation_time * 5), Vector3d(0, 0, 1))).matrix();

								s[8] = model_transform;// base point of second level legs for each (top of first leg level box)
									
									model_transform *= Affine3d(Translation3d(0, -.75/2, 0)).matrix();
									model_transform *= Vector4d(0.2, .75, .2, 1).asDiagonal();
									m_cube->draw(projection_transform, camera_transform, model_transform, "");
									axis->draw(basis_id++, projection_transform, camera_transform, model_transform, "");

								model_transform = s[8];


							model_transform = s[9];
						}

					//back to the center of leg hinges point(bottom of bee body)
					model_transform = s[5];

					//left leg first level hinge
					model_transform *= Affine3d(Translation3d(-.2, 0, 0)).matrix();
					s[5] = model_transform; //
						//draw legs
						for (int i = 0; i < 3; i++)
						{

							s[9] = model_transform;

								//first level leg(hinge for first level leg)
								model_transform *= Affine3d(AngleAxisd(-PI / 3 + 0.4* sin(animation_time * 5), Vector3d(0, 0, 1))).matrix();
								model_transform *= Affine3d(Translation3d(0, -.4 / 2, (i - 1) / 2.0)).matrix();
								s[8] = model_transform;// base point of first level legs for each

									model_transform *= Vector4d(0.2, .4, .2, 1).asDiagonal();
									m_cube->draw(projection_transform, camera_transform, model_transform, "");
									axis->draw(basis_id++, projection_transform, camera_transform, model_transform, "");

								model_transform = s[8];

								//second level leg(hinge point for second level legs)
								model_transform *= Affine3d(Translation3d(0, -.4 / 2, 0)).matrix();
								model_transform *= Affine3d(AngleAxisd(PI / 3 + 0.4* sin(animation_time * 5), Vector3d(0, 0, 1))).matrix();

								s[8] = model_transform;// base point of second level legs for each (top of first leg level box)

									model_transform *= Affine3d(Translation3d(0, -.75 / 2, 0)).matrix();
									model_transform *= Vector4d(0.2, .75, .2, 1).asDiagonal();
									m_cube->draw(projection_transform, camera_transform, model_transform, "");
									axis->draw(basis_id++, projection_transform, camera_transform, model_transform, "");

								model_transform = s[8];

							model_transform = s[9];
						}
						model_transform = s[5];
	}


	virtual void update_debug_strings( std::map< std::string, std::string >&	 debug_screen_strings )		// Strings this particular class contributes to the UI
	{ 
		debug_screen_strings["animation time:"] = "Animation time:  " + std::to_string( animation_time ); 
		debug_screen_strings["showing basis "] = "Showing basis " + std::to_string( axis->basis_selection );
		debug_screen_strings["animate:"] = "Animation " + string( animate ? "on" : "off" ); 
		debug_screen_strings["zthrust"] = "Z thrust:  " + std::to_string( thrust[2] );
	}
	
	void forward() { thrust[2] =  1; }  void back()  { thrust[2] = -1; }    void up()    { thrust[1] = -1; }
	void left()    { thrust[0] =  1; }  void right() { thrust[0] = -1; }    void down()  { thrust[1] =  1; }
	void stopX()   { thrust[0] =  0; }  void stopY() { thrust[1] =  0; }    void stopZ() { thrust[2] =  0; }
	void toggleLooking() { looking = !looking; }
	void next_basis() { axis->basis_selection++; }	void prev_basis() { axis->basis_selection--; }
	void toggleColorNormals() { color_normals = !color_normals; glUniform1i( g_addrs->COLOR_NORMALS_loc, color_normals); }
	void toggleGouraud() { gouraud = !gouraud; glUniform1i( g_addrs->GOURAUD_loc, gouraud);}
	void toggleAnimate() { animate = !animate; }
	void reset() { camera_transform = Matrix4d::Identity(); }
	void roll_left() { camera_transform  *= Affine3d( AngleAxisd( 3 * PI /180, Vector3d( 0, 0,  1 ) ) ).matrix(); }
	void roll_right() { camera_transform *= Affine3d( AngleAxisd( 3 * PI /180, Vector3d( 0, 0, -1 ) ) ).matrix(); }
	void toggleSolid() { solid = !solid; glUniform1i( g_addrs->SOLID_loc, solid); glUniform4fv( g_addrs->SOLID_COLOR_loc, 1, Vector4f(Vector4f::Random()).data() );}
	
	// *******************************************************	
	// init_keys():  Define any extra keyboard shortcuts here
	virtual	void init_keys() 
	{
		keys.addHandler		( 'w', 0,					Callback<Animation>( &Animation::forward , this ) );
		keys.addHandler		( 'a', 0,					Callback<Animation>( &Animation::left    , this ) );
		keys.addHandler		( 's', 0,					Callback<Animation>( &Animation::back    , this ) );
		keys.addHandler		( 'd', 0,					Callback<Animation>( &Animation::right   , this ) );
		keys.addHandler		( ' ', 0,					Callback<Animation>( &Animation::up      , this ) );
		keys.addHandler		( 'z', 0,					Callback<Animation>( &Animation::down    , this ) );
		
		keys.addHandler		( 'w', GLUT_Port::RELEASE,  Callback<Animation>( &Animation::stopZ , this ) );
		keys.addHandler		( 'a', GLUT_Port::RELEASE,  Callback<Animation>( &Animation::stopX , this ) );
		keys.addHandler		( 's', GLUT_Port::RELEASE,  Callback<Animation>( &Animation::stopZ , this ) );
		keys.addHandler		( 'd', GLUT_Port::RELEASE,  Callback<Animation>( &Animation::stopX , this ) );
		keys.addHandler		( ' ', GLUT_Port::RELEASE,  Callback<Animation>( &Animation::stopY , this ) );
		keys.addHandler		( 'z', GLUT_Port::RELEASE,  Callback<Animation>( &Animation::stopY , this ) );
		
		keys.addHandler		( '.', 0,                   Callback<Animation>( &Animation::roll_left , this ) );
		keys.addHandler		( ',', 0,                   Callback<Animation>( &Animation::roll_right , this ) );
		keys.addHandler		( 'r', 0,                   Callback<Animation>( &Animation::reset , this ) );
		keys.addHandler		( 'f', 0,                   Callback<Animation>( &Animation::toggleLooking , this ) );
		keys.addHandler		( 'p', 0,                   Callback<Animation>( &Animation::next_basis , this ) );
		keys.addHandler		( 'm', 0,                   Callback<Animation>( &Animation::prev_basis , this ) );
		
		keys.addHandler		( 'n', GLUT_Port::ALT,      Callback<Animation>( &Animation::toggleColorNormals , this ) );
		keys.addHandler		( 'g', GLUT_Port::ALT,      Callback<Animation>( &Animation::toggleGouraud , this ) );
		keys.addHandler		( 's', GLUT_Port::ALT,      Callback<Animation>( &Animation::toggleSolid , this ) );
		keys.addHandler		( 'a', GLUT_Port::ALT,      Callback<Animation>( &Animation::toggleAnimate , this ) );

		update_controls< Animation >( keys );
	}
	// This must go in every class that has its own KeyMap and keyboard shortcuts
	virtual void handle_key( unsigned char key, unsigned char mods )	{	if( keys.hasHandler( key, mods ) )		keys.getHandler( key, mods )	();		}
	
};

int main()
{
	Animation a;
	while(1)
	{
		glutMainLoopEvent();		// Run the OpenGL event loop forever
		GLUT_Port::idle();
	}
}