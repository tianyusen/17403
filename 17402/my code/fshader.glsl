// *******************************************************
// CS 174a Graphics Example Code
// The following string is loaded by our C++ and then used as the Fragment Shader program, which gets sent to the graphics card at runtime.  The fragment shader runs once
// all vertices in a triangle / element finish their vertex shader programs, and thus have finished finding out where they land on the screen.  The fragment shader fills 
// in (shades) every pixel (fragment) overlapping where the triangle landed.  At each pixel it interpolates different values from the three extreme points of the triangle, and 
// uses them in formulas to determine color.

precision mediump float;

uniform vec4 lightColor, color;
uniform float ambient, chalkiness, shininess, smoothness;

varying vec2 fTexCoord;		// per-fragment interpolated values from the vertex shader
varying vec3 N, L, E, pos;

uniform sampler2D texture; 
uniform bool SOLID, GOURAUD, COLOR_NORMALS, COLOR_VERTICES, USE_TEXTURE;
varying vec4 VERTEX_COLOR;

void main()
{    
	if( SOLID || GOURAUD || COLOR_NORMALS )
	{
		gl_FragColor = VERTEX_COLOR;
		return;
	}
	vec3 H = normalize( L + E );

	float diffuse  = max( dot(L, N), 0.0 );
	float specular = pow( max(dot(N, H), 0.0), smoothness );
	
	vec4 tex_color = texture2D( texture, fTexCoord );
	if( tex_color.w < .2 && USE_TEXTURE ) discard;
	
	gl_FragColor =  (USE_TEXTURE ? tex_color : vec4(1,1,1,1)) * color * (  ambient + chalkiness * diffuse ) + lightColor * ( shininess * specular );
	gl_FragColor.a = gl_FragColor.w;
}