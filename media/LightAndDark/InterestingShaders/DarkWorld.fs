uniform sampler2D baseMap;
uniform sampler2D normalMap;
uniform sampler2D heightMap;
uniform bool useHeightMap;

varying vec3 v_lightVector;
varying vec3 v_viewVector;

void main()
{
	//determine if we are going to use the height map
	float height = 0.;
	float v = 0.;
	/*if( useHeightMap )
	{
		height = texture2D( heightMap, gl_TexCoord[ 0 ].st ).r;
		vec2 scaleBias = vec2( 0.06, 0.03 );
		v = height * scaleBias.s - scaleBias.t;
	}*/

	vec3 V = normalize( v_viewVector );
	vec2 texCoords = gl_TexCoord[ 0 ].st + ( V.xy * v );

	//
	float bumpiness = 1.0;
	vec3 smoothOut = vec3( 0.5, 0.5, 1.0 );
	vec3 N = texture2D( normalMap, texCoords ).rgb;
	//N = mix( smoothOut, N, bumpiness );
	N = mix(smoothOut, N);	// This change causes a dark, errie scene.
	N = normalize( ( N * 2.0 ) - 1.0 );

	//
	vec3 L = normalize( v_lightVector );
	float NdotL = max( dot( N, L ), 0.0 );

	//
	vec3 R = reflect( V, N );
	float RdotL = max( dot( R, L ), 0.0 );

	//
	float specularPower = 50.0;
	vec3 base = texture2D( baseMap, texCoords ).rgb;
	vec3 ambient = vec3( 0.368627, 0.368421, 0.368421 ) * base;
	//vec3 ambient = vec3( 0.886275, 0.886275, 0.886275 ) * base;
	vec3 diffuse = vec3( 0.886275, 0.885003, 0.885003 ) * base * NdotL;
	vec3 specular = vec3( 0.490196, 0.488722, 0.488722 ) * pow( RdotL, specularPower );
	vec3 color = ambient + diffuse + specular;

	//
	gl_FragColor = vec4( color, 1.0 );
}
