#pragma once
#include "../CS174a template/Utilities.h"

// *******************************************************
// CS 174a Graphics Example Code
// Shape.cpp - Defines a number of objects that inherit from the Shape class.  Each manages lists of its own vertex positions, vertex normals, and texture coordinates per vertex.  
// Upon shape initialization, OpenGL functions are called to pass each list into a buffer in the graphics card's memory.


struct Shape
	{
	// *******************************************************
	// IMPORTANT: When you extend the Shape class, these are the four arrays you must put values into.  Each shape has a list of vertex positions (here just called vertices), vertex normals 
	// (vectors that point away from the surface for each vertex), texture coordinates (pixel coordintates in the texture picture, scaled down to the range [ 0.0, 1.0 ] to place each vertex 
	// somewhere relative to the picture), and most importantly - indices, a list of index triples defining which three vertices belong to each triangle.  Your class must build these lists
	// and then send them to the graphics card by calling init_buffers().  At some point a simple example will be given of manually building these lists for a shape.

		std::vector < Vector3d > vertices, normals;
		std::vector < Vector2d > texture_coords;
		std::vector < unsigned > indices;
		bool indexed;
		GLuint position_buffer, normal_buffer, texture_coord_buffer, vertex_color_buffer, index_buffer;		// Memory addresses of the buffers given to this shape in the graphics card.
		
		Shape() : indexed(true) { }

		void flat_normals_from_triples( unsigned offset )		// This calculates normals automatically for flat shaded elements, assuming that each element is independent (no shared vertices)
		{
			normals.resize( vertices.size() );
			for( unsigned counter = offset; counter < ( indexed ? indices.size() : vertices.size() ) ; counter += 3 )
			{
				Vector3d a = vertices[ indexed ? indices[ counter     ] : counter     ] ;
				Vector3d b = vertices[ indexed ? indices[ counter + 1 ] : counter + 1 ] ;
				Vector3d c = vertices[ indexed ? indices[ counter + 2 ] : counter + 2 ] ;
	
				Vector3d triangleNormal = ( a - b ).cross( c - a ).normalized();	// Cross two edge vectors of this triangle together to get the normal
				if( ( triangleNormal + a ).norm() < a.norm() )
						triangleNormal *= -1;		// Flip the normal if for some point it brings us closer to the origin
				
				if( triangleNormal[0] != triangleNormal[0] || triangleNormal[1] != triangleNormal[1] ||  triangleNormal[2] != triangleNormal[2] )
				{																	// Did we divide by zero?  Handle it
					normals[ indices[ counter     ] ] = Vector3d( 0, 0, 1 );		
					normals[ indices[ counter + 1 ] ] = Vector3d( 0, 0, 1 );
					normals[ indices[ counter + 2 ] ] = Vector3d( 0, 0, 1 );
					return;
				}
				normals[ indices[ counter     ] ] = Vector3d( triangleNormal[0], triangleNormal[1], triangleNormal[2] );
				normals[ indices[ counter + 1 ] ] = Vector3d( triangleNormal[0], triangleNormal[1], triangleNormal[2] );
				normals[ indices[ counter + 2 ] ] = Vector3d( triangleNormal[0], triangleNormal[1], triangleNormal[2] );
			}
		}

		void spherical_texture_coords ( unsigned vert_index )
		{	texture_coords.push_back( Vector2d( .5 + atan2( vertices[vert_index][2], vertices[vert_index][0] ) / 2 / PI, .5 - 2 * asin( vertices[vert_index][1] ) / 2 / PI ) );
		}

		template < class stlVector > void flatten( std::vector< float > &buffer, stlVector eigenObjects )
		{
			buffer.clear();
			for( auto it = eigenObjects.begin(); it != eigenObjects.end(); it++ )
				for( int i = 0; i < it->size() ; i++ )
					buffer.push_back( float( ( *it ) [i]) );
			glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW );	
		}
		
		void init_buffers()		// Send the completed vertex and index lists to their own buffers in the graphics card.
		{
			std::vector< float > buffer;
			glGenBuffers( 1, &position_buffer );
			glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
			flatten( buffer, vertices );
			
			glGenBuffers( 1, &normal_buffer );
			glBindBuffer( GL_ARRAY_BUFFER, normal_buffer);
			flatten( buffer, normals );	
			
			glGenBuffers( 1, &texture_coord_buffer );
			glBindBuffer( GL_ARRAY_BUFFER, texture_coord_buffer);
			flatten( buffer, texture_coords );

			glGenBuffers( 1, &vertex_color_buffer );
			
			if( indexed )
			{
				glGenBuffers( 1, &index_buffer );
				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer);
				glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW );
			}
		}
	
		void update_uniforms( const Matrix4d& projection_transform, const Matrix4d& camera_transform, const Matrix4d& model_transform )		// Send the current matrices to the shader
		{
			Matrix4f camera_transform_float				= camera_transform.cast<float>();
			Matrix4d camera_model_transform 			= camera_transform * model_transform;
			Matrix4f camera_model_transform_float 		= camera_model_transform.cast<float>();
			Matrix4f projection_camera_model_transform 	= ( projection_transform * camera_model_transform ).cast<float>();
			Matrix3f camera_model_transform_normal		= camera_model_transform.inverse().transpose().topLeftCorner<3, 3>().cast<float>();
				
			glUniformMatrix4fv( g_addrs->camera_transform_loc, 					1, GL_FALSE, camera_transform_float.data() );
			glUniformMatrix4fv( g_addrs->camera_model_transform_loc, 			1, GL_FALSE, camera_model_transform_float.data() );
			glUniformMatrix4fv( g_addrs->projection_camera_model_transform_loc,  1, GL_FALSE, projection_camera_model_transform.data() );
			glUniformMatrix3fv( g_addrs->camera_model_transform_normal_loc,		1, GL_FALSE, camera_model_transform_normal.data() );
				   
			glUniform4fv( g_addrs->lightPosition_loc, 	1, Vector4f(  10,7,3,0 ).data() );
			glUniform4fv( g_addrs->lightColor_loc, 		1, Vector4f(  1,1,1,1 ).data() );
			glUniform1f ( g_addrs->ambient_loc,  1 );
			glUniform1f ( g_addrs->diffuse_loc,  1 );
			glUniform1f ( g_addrs->specular_loc, 1 );
			glUniform1f ( g_addrs->shininess_loc, 40 );
		}

		// The same draw call is used for every shape - the calls draw different things due to the different vertex lists we stored in the graphics card for them.
		void draw( const Matrix4d &projection_transform, const Matrix4d &camera_transform, const Matrix4d &model_transform, const std::string &texture_filename )
		{
			update_uniforms( projection_transform, camera_transform, model_transform );
			glBindBuffer( GL_ARRAY_BUFFER, position_buffer);
			glVertexAttribPointer( g_addrs->vPosition, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
			
			glBindBuffer( GL_ARRAY_BUFFER, normal_buffer);
			glVertexAttribPointer( g_addrs->vNormal, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
			
			if( texture_filename.length() )		// Use an empty string parameter to signal that we don't want to texture this shape.
			{
				glEnableVertexAttribArray( g_addrs->vTexCoord );	
				glUniform1f ( g_addrs->USE_TEXTURE_loc,  1 );
				glBindTexture( GL_TEXTURE_2D, textures[texture_filename]->id );
				glBindBuffer( GL_ARRAY_BUFFER, texture_coord_buffer   );
				glVertexAttribPointer( g_addrs->vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
			}
			else
				{	glDisableVertexAttribArray( g_addrs->vTexCoord );	glUniform1f ( g_addrs->USE_TEXTURE_loc,  0 );	}

			if( indexed )			
			{
				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer );
				glDrawElements( GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (GLvoid*)0 );
			}
			else
				glDrawArrays  ( GL_TRIANGLES, 0, vertices.size() );
		}
	};


struct Triangle_Fan_Full : public Shape		// Arrange triangles in a fan.  This version goes all the way around a circle with them.
	{	
		Triangle_Fan_Full( const Matrix4d &points_transform )
		{
			populate( *this, 10, points_transform, -1 );
			init_buffers();
		}
	private:
		static void createCircleVertices ( Shape &recipient, unsigned num_tris ) 
			{	for( unsigned counter = 0; counter <= num_tris; counter++  )
				{
					recipient.vertices.push_back( Vector3d( cos(2 * PI * counter/num_tris), sin(2 * PI * counter/num_tris), -1 ) );	
					recipient.texture_coords.push_back( Vector2d( 1. * counter/num_tris, 1 ) );	
				}
			}
		
		static void initFromSequence ( Shape &recipient, unsigned center_idx, unsigned num_tris, unsigned offset )
			{	
				for( unsigned index = offset; index <= offset + num_tris;	 )
				{
					recipient.indices.push_back( index );
					recipient.indices.push_back( center_idx );
					recipient.indices.push_back( ++index );
				}
				recipient.indices.back() = offset;
			}
	public: 
		static void populate( Shape &recipient, unsigned num_tris, const Matrix4d &points_transform, unsigned center_idx )
			{
				if( center_idx == -1 )			// Not re-using a point?  Create one.
				{
					recipient.vertices.push_back( ( points_transform * Vector4d( 0,0,1,1 ) ).topRows<3>() );
					center_idx = recipient.vertices.size() - 1;
					recipient.texture_coords.push_back( Vector2d( 1, 0 ) );
				}				
				unsigned offset = recipient.vertices.size();		unsigned index_offset = recipient.indices.size();				
			
				createCircleVertices( recipient, num_tris );
				initFromSequence(	  recipient, center_idx, num_tris, offset );
			
				recipient.flat_normals_from_triples( index_offset );	
		
				for( unsigned i = offset; i < recipient.vertices.size(); i++ )
					recipient.vertices[i] = ( points_transform * (Vector4d() << recipient.vertices[ i ], 1).finished() ).topRows<3>();	
			}
	};

struct Triangle_Strip : public Shape		// Arrange triangles in a strip, where the list of vertices alternates sides.
{	
	static void init_from_strip_lists( Shape &recipient, std::vector < Vector3d > &vertices, std::vector < unsigned > &indices )
	{
		unsigned offset = recipient.vertices.size();
			
		for( unsigned counter = 0; counter < vertices.size(); 		)
			recipient.vertices.push_back ( vertices[ counter++ ] );
			
		for( unsigned counter = 0; counter < indices.size() - 2; counter++ )
		{																		// The modulus, used as a conditional here, makes face orientations uniform.
			recipient.indices.push_back( indices[counter + 2 * ((counter+1) % 2 ) ] + offset );		
			recipient.indices.push_back( indices[counter + 1] + offset );
			recipient.indices.push_back( indices[counter + 2 * ( counter    % 2 ) ] + offset );
		}	
	}
};

struct Rectangular_Strip : public Triangle_Strip
{
	Rectangular_Strip( unsigned numRectangles, const Matrix4d &points_transform)
	{
		populate( *this, numRectangles, points_transform );
		init_buffers();
	}

	static void populate( Shape &recipient, unsigned numRectangles, const Matrix4d &points_transform )
		{
			unsigned offset = recipient.vertices.size(),	 index_offset = recipient.indices.size(),
			topIdx = 0,			bottomIdx = numRectangles + 1;
			std::vector < Vector3d > vertices( (numRectangles + 1 ) * 2 );
			std::vector < unsigned > strip_indices;		
			recipient.texture_coords.resize( recipient.texture_coords.size() + (numRectangles + 1 ) * 2 );
			for( unsigned i = 0; i <= numRectangles; i++ )
			{
				vertices[topIdx] 	= Vector3d( 0,  .5, topIdx - .5 * numRectangles );		
					recipient.texture_coords[ topIdx + offset ]    = Vector2d( 1. * topIdx / numRectangles, 1 );
				vertices[bottomIdx] = Vector3d( 0, -.5, topIdx - .5 * numRectangles );		
					recipient.texture_coords[ bottomIdx + offset ] = Vector2d( 1. * topIdx / numRectangles, 0 );
				strip_indices.push_back(topIdx++);
				strip_indices.push_back(bottomIdx++);
			}
					
			init_from_strip_lists(recipient, vertices, strip_indices);
							
			for( unsigned i = offset; i < recipient.vertices.size(); i++ )
				recipient.vertices[i] = ( points_transform * ( Vector4d() << recipient.vertices[i], 1 ).finished() ).topRows<3>();	
			recipient.flat_normals_from_triples( index_offset );
		} 
};



struct Text_Line : public Shape		// Draws a rectangle textured with images of ASCII characters over each quad, spelling out a string.
{
	unsigned max_size;
	Text_Line( unsigned max_size, const Matrix4d &points_transform ) : max_size(max_size)
	{
		Matrix4d object_transform = points_transform;
		for( unsigned i = 0; i < max_size; i++ )
		{
			Rectangular_Strip::populate( *this, 1, object_transform );
			object_transform *= Affine3d( Translation3d(0, 0, -.7)).matrix();
		}
		init_buffers();
	}

	void set_string( string line )
	{
		for( unsigned i = 0; i < max_size; i++ )
		{
			unsigned row = ( i < line.size() ? line[i] : ' ' ) / 16,
			         col = ( i < line.size() ? line[i] : ' ' ) % 16;
				
			unsigned skip = 3, size = 32, sizefloor = size - skip;
			float dim = size * 16.f, left  = (col * size + skip) / dim, 		top    = (row * size + skip) / dim, 
									right = (col * size + sizefloor) / dim, 	bottom = (row * size + sizefloor + 5) / dim;
			
			texture_coords[ 4 * i ]		= Vector2d( right, top );
			texture_coords[ 4 * i + 1 ] = Vector2d( left, top );
			texture_coords[ 4 * i + 2 ] = Vector2d( right, bottom );
			texture_coords[ 4 * i + 3 ] = Vector2d( left, bottom );
		}

		std::vector< float > buffer;
		glBindBuffer( GL_ARRAY_BUFFER, texture_coord_buffer);
		flatten( buffer, texture_coords );
	}

	void draw_heads_up_display( const Matrix4d &projection_transform, const Matrix4d &camera_transform, const Matrix4d &model_transform )
	{
		glDisable( GL_DEPTH_TEST );
		Shape::draw(projection_transform, camera_transform, model_transform, "text.tga" );	
		glEnable(  GL_DEPTH_TEST );
	}
};





struct Cylindrical_Strip : public Triangle_Strip
{
	Cylindrical_Strip( unsigned numRectangles, const Matrix4d &points_transform )
	{
		populate( *this, numRectangles, points_transform );
		init_buffers();
	}

	static void populate( Shape &recipient, unsigned numRectangles, const Matrix4d &points_transform )	
	{	
		std::vector < Vector3d > vertices( numRectangles * 2 );
		std::vector < unsigned > strip_indices;
		unsigned offset = recipient.vertices.size(),	 index_offset = recipient.indices.size(),
			topIdx = 0,			bottomIdx = numRectangles;
						
		recipient.texture_coords.resize( recipient.texture_coords.size() + numRectangles * 2 );
		for( unsigned i = 0; i < numRectangles; i++ )
		{
			vertices[topIdx] 	= Vector3d( cos(2 * PI * topIdx / numRectangles), sin(2 * PI * topIdx / numRectangles), .5 );	
			recipient.texture_coords[topIdx + offset]    = Vector2d(0, 1. * topIdx / numRectangles );
			vertices[bottomIdx] = Vector3d( cos(2 * PI * topIdx / numRectangles), sin(2 * PI * topIdx / numRectangles), -.5 );			
			recipient.texture_coords[bottomIdx + offset] = Vector2d(1, 1. * topIdx / numRectangles );
			strip_indices.push_back(topIdx++);
			strip_indices.push_back(bottomIdx++);
		}
		strip_indices.push_back( 0 );
		strip_indices.push_back( numRectangles );
								
		init_from_strip_lists(recipient, vertices, strip_indices);
							
		for( unsigned i = offset; i < recipient.vertices.size(); i++ )
			recipient.vertices[i] = ( points_transform * (Vector4d() << recipient.vertices[i], 1 ).finished() ).topRows<3>() ;	
		recipient.flat_normals_from_triples( index_offset );
	}
};




struct Cube : public Shape
{
	Cube( const Matrix4d &points_transform )
	{
		populate( *this, points_transform );
		init_buffers();
	}
	static void populate( Shape &recipient, const Matrix4d &points_transform )	
	{	
		for( unsigned i = 0; i < 3; i++ )			// Build a cube by inserting six triangle strips into the lists.
				for( unsigned j = 0; j < 2; j++ )
					Rectangular_Strip::populate( recipient, 1, points_transform * 
						Affine3d( AngleAxisd( PI / 2, Vector3d( i==0, i==1, i==2 ) ) ).matrix() *  Affine3d(Translation3d( j - .5, 0, 0 ) ).matrix() );
	}
};

// Build a sphere using subdivision, starting with a tetrahedron.  Store each level of detail in separate index lists.
struct Sphere : public Shape
{	
	std::vector < std::vector < unsigned > > indices_LOD;
	GLuint* index_buffer_LOD;
	Sphere( const Matrix4d &points_transform, unsigned max_subdivisions ) : indices_LOD( max_subdivisions + 1 )
	{
		vertices.push_back(		Vector3d(0.0, 0.0, -1.0) 				 );
		vertices.push_back(		Vector3d(0.0, 0.942809, 0.333333) 		 );
		vertices.push_back(		Vector3d(-0.816497, -0.471405, 0.333333) );
		vertices.push_back(		Vector3d(0.816497, -0.471405, 0.333333)  );
			
		subdivideTriangle( 0, 1, 2, max_subdivisions);
		subdivideTriangle( 3, 2, 1, max_subdivisions);
		subdivideTriangle( 1, 0, 3, max_subdivisions);
		subdivideTriangle( 0, 2, 3, max_subdivisions); 
			
		for( unsigned i = 0; i < vertices.size(); i++ )
		{
			spherical_texture_coords( i );
			normals.push_back( vertices[i] );	
		}

		index_buffer_LOD = new GLuint[max_subdivisions + 1];
		glGenBuffers( max_subdivisions, index_buffer_LOD + 1 );

		for( unsigned i = 1; i <= max_subdivisions; i++ )
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_LOD[ i ] );
			glBufferData( GL_ELEMENT_ARRAY_BUFFER,  indices_LOD[i].size() * sizeof(unsigned), indices_LOD[i].data(), GL_STATIC_DRAW );
		}
			
		init_buffers();
	}
		
	void populate( Shape &recipient, const Matrix4d &points_transform, unsigned LOD )
	{
		unsigned offset = recipient.vertices.size();
		for( unsigned i = 0; i < vertices.size(); i++ )
		{	
			recipient.normals .push_back( vertices[i] );	
			recipient.vertices.push_back( ( points_transform * (Vector4d() << vertices[i], 1 ).finished() ).topRows<3>() );
			recipient.spherical_texture_coords( i );
		}
		std::vector<unsigned> &this_LOD = LOD ? indices_LOD[ LOD ] : indices;
		for( unsigned i = 0; i < indices.size(); i++ )
			recipient.indices.push_back( this_LOD[i] + offset );
	}

	void draw( const Matrix4d &projection_transform, const Matrix4d &camera_transform, const Matrix4d &model_transform, 
		const std::string &texture_filename, int LOD )
	{ 	
		update_uniforms( projection_transform, camera_transform, model_transform );
		glBindBuffer( GL_ARRAY_BUFFER, position_buffer);
		glVertexAttribPointer( g_addrs->vPosition, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
			
		glBindBuffer( GL_ARRAY_BUFFER, normal_buffer);
		glVertexAttribPointer( g_addrs->vNormal, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
			
		if( texture_filename.length() )
		{
			glEnableVertexAttribArray( g_addrs->vTexCoord );	
			glUniform1f ( g_addrs->USE_TEXTURE_loc,  1 );
			glBindTexture( GL_TEXTURE_2D, textures[texture_filename]->id );
			glBindBuffer( GL_ARRAY_BUFFER, texture_coord_buffer   );
			glVertexAttribPointer( g_addrs->vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
		}
		else
			{	glDisableVertexAttribArray( g_addrs->vTexCoord );	glUniform1f ( g_addrs->USE_TEXTURE_loc,  0 );	}

		if( LOD < 0 || LOD + 1 >= (int)indices_LOD.size() )
		{
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer );
			glDrawElements( GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (GLvoid*)0 );
		}
		else
		{
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer_LOD[ indices_LOD.size() - 1 - LOD ] );
			glDrawElements( GL_TRIANGLES, indices_LOD[ indices_LOD.size() - 1 - LOD ].size(), GL_UNSIGNED_INT, (GLvoid*)0 );
		}			
	}		
private:
	void subdivideTriangle( unsigned a, unsigned b, unsigned c, int count ) 
	{	
		( count ? indices_LOD[count] : indices ).push_back(a);
		( count ? indices_LOD[count] : indices ).push_back(b);
		( count ? indices_LOD[count] : indices ).push_back(c);
		if( !count ) return;	// Build index list with the nice property that skipping every fourth vertex index takes you down one level of detail, each time				
		Vector3d ab_vert = ( vertices[a] + vertices[b] ).normalized();
		Vector3d ac_vert = ( vertices[a] + vertices[c] ).normalized();
		Vector3d bc_vert = ( vertices[b] + vertices[c] ).normalized();	
						
		unsigned ab = vertices.size();		vertices.push_back( ab_vert );	
		unsigned ac = vertices.size();		vertices.push_back( ac_vert );
		unsigned bc = vertices.size();		vertices.push_back( bc_vert );	

		subdivideTriangle( a, ab, ac,  count - 1 );
		subdivideTriangle( ab, b, bc,  count - 1 );
		subdivideTriangle( ac, bc, c,  count - 1 );
		subdivideTriangle( ab, bc, ac, count - 1 );
	}
};

struct Axis : public Shape
{
	int basis_selection;
	Axis( const Matrix4d &points_transform ) : basis_selection(0)
	{
		populate( *this, points_transform );
		init_buffers();
	}
	// Only draw this set of axes if it is the one selected through the user interface.
	void draw( int current, const Matrix4d &projection_transform, const Matrix4d &camera_transform, const Matrix4d &model_transform, const std::string &texture_filename )
		{	if( basis_selection == current ) Shape::draw(projection_transform, camera_transform, model_transform, texture_filename );	}

	static void populate( Shape &recipient, const Matrix4d &points_transform )	
	{
		Matrix4d object_transform = Matrix4d::Identity();
		object_transform *= Vector4d( .25, .25, .25, 1 ).asDiagonal();
		(new Sphere( Matrix4d::Identity(), 3 ) )->populate( recipient, object_transform, 0 );
		object_transform = Matrix4d::Identity();
		drawOneAxis(recipient, object_transform);
		object_transform *= Affine3d( AngleAxisd( -PI/2, Vector3d( 1, 0, 0 ) ) ).matrix();
		object_transform *= Vector4d( 1, -1, 1, 1 ).asDiagonal();
		drawOneAxis(recipient, object_transform);
		object_transform *= Affine3d( AngleAxisd( PI/2, Vector3d( 0, 1, 0 ) ) ).matrix();
		object_transform *= Vector4d( -1, 1, 1, 1 ).asDiagonal();
		drawOneAxis(recipient, object_transform);
	}
private:	
	static void drawOneAxis( Shape &recipient, const Matrix4d &points_transform )
	{
		Matrix4d original(points_transform), object_transform(points_transform);
		object_transform *= Affine3d(Translation3d( 0, 0, 4 )).matrix();
		object_transform *= Vector4d( .25, .25, .25, 1 ).asDiagonal();
		Triangle_Fan_Full::populate ( recipient, 10, object_transform, -1 );
		object_transform = original;
		object_transform *= Affine3d(Translation3d( 1, 1, .5 )).matrix();
		object_transform *= Vector4d( .1, .1, 1, 1 ).asDiagonal();
		Cube::populate( recipient, object_transform );
		object_transform = original;
		object_transform *= Affine3d(Translation3d( 1, 0, .5 )).matrix();
		object_transform *= Vector4d( .1, .1, 1, 1 ).asDiagonal();
		Cube::populate( recipient, object_transform );
		object_transform = original;
		object_transform *= Affine3d(Translation3d( 0, 1, .5 )).matrix();
		object_transform *= Vector4d( .1, .1, 1, 1 ).asDiagonal();
		Cube::populate( recipient, object_transform );
		object_transform = original;			
		object_transform *= Affine3d(Translation3d( 0, 0, 2 )).matrix();
		object_transform *= Vector4d( .1, .1, 4, 1 ).asDiagonal();
		Cylindrical_Strip::populate( recipient, 7, object_transform );
	}
};
