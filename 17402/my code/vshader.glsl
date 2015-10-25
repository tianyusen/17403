// *******************************************************
// CS 174a Graphics Example Code
// The following string is loaded by our C++ and then used as the Vertex Shader program.  Our C++ sends this code to the graphics card at runtime, where 
// on each run it gets compiled and linked there.  Thereafter, all of your calls to draw shapes will launch the vertex shader program once per vertex in the
// shape (three times per triangle), sending results on to the next phase.  The purpose of this program is to calculate the final resting place of vertices in 
// screen coordinates; each of them starts out in local object coordinates.

precision mediump float;

attribute vec4 vColor;
attribute vec3 vPosition, vNormal;
attribute vec2 vTexCoord;
varying vec2 fTexCoord;
varying vec3 N, L, E, pos;

uniform bool SOLID, GOURAUD, COLOR_NORMALS, COLOR_VERTICES;
uniform vec4 SOLID_COLOR;
varying vec4 VERTEX_COLOR;

uniform mat4 camera_transform, camera_model_transform, projection_camera_model_transform;
uniform mat3 camera_model_transform_normal;

uniform vec4 lightPosition, lightColor, color;
uniform float ambient, chalkiness, shininess, smoothness;

void main()
{
    N = normalize( camera_model_transform_normal * vNormal );
	
	vec4 object_space_pos = vec4(vPosition, 1.0);
    gl_Position = projection_camera_model_transform * object_space_pos;

	if( SOLID || COLOR_NORMALS || COLOR_VERTICES )
	{
		VERTEX_COLOR   = SOLID ? SOLID_COLOR : ( COLOR_NORMALS ? abs( vec4( N, 1.0 ) ) : vColor );
		VERTEX_COLOR.a = VERTEX_COLOR.w;
		return;
	}
	
    pos = ( camera_model_transform * object_space_pos ).xyz;
	E = -pos;
    L = normalize( ( camera_transform * lightPosition ).xyz - lightPosition.w * pos );		// Use w = 0 for a directional light -- a vector instead of a point.    

	if( GOURAUD )
	{
		vec3 H = normalize( L + E );

		float diffuse  = max( dot(L, N), 0.0 );
		float specular = pow( max(dot(N, H), 0.0), smoothness );

		VERTEX_COLOR = color * ( ambient + chalkiness * diffuse ) + lightColor * ( shininess * specular );
		VERTEX_COLOR.a = VERTEX_COLOR.w;
	}  
	fTexCoord = vTexCoord;  
}