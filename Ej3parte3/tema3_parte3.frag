#version 150 core

struct LightInfo {
		vec4 lightPos; // S.R. del mundo
		vec3 intensity;
	};

struct MaterialInfo {
		vec3 ambient;
		vec3 diffuse;
		vec3 specular;
		float shininess;
	};

uniform LightInfo uLight;
uniform MaterialInfo uMaterial;
uniform sampler2D uNormalMap;

uniform sampler2D uPerlinMap;

uniform vec3 uLightPosition;
uniform vec3 uLInten;
uniform vec3 uAmbM;
uniform vec3 uDiffM;
uniform vec3 uSpecM;
uniform float uSh = 128.0;
uniform bool use_normal;

uniform int p[512];
uniform int epsilon;


in vec3 vEcPos;
in vec3 vEcNorm;
in vec3 vPosition;
in vec3 vLightDir;
in vec3 vViewDir;
in vec3 vNormal;
in vec2 vST;

vec2 Vst = vST;

out vec4 fFragColor;

const float noise_factor = 50;
const float noise_factor2 = 4;
const float noise_factor3 = 8;

/*
*
*
* ILUMINACION POR FRAGMENTO
*
*/
float noise(vec2);

float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))
                 * 43758.5453123);
}

float fade(float t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

float lerp(float t, float a, float b) { 
	return a + t * (b - a); 
}

float grad(int hash, float x, float y, float z) {
	int h = hash & 15;
	float u = h < 8 ? x : y;
	float v;

	if (h<4) {
		v = y;
	} else if (h==12||h==14) {
		v = x;
	} else {
		v = z;
	}
		
	return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v); 
}

vec3 ads (vec3 _normal, vec3 _vViewDir, vec3 ldir) {
	vec3 r = reflect(-ldir,_normal);
	vec3 specular;
	specular = vec3(0.5);

	// // if (use_color)
	// // 	return uLight.intensity * (vec3(texture(uPerlinMap, Vst)) * 0.2 + vec3(texture(uPerlinMap, Vst)) * max(dot(ldir,_normal), 0.0) +  specular * pow(max(dot(r,_vViewDir),0), uMaterial.shininess));
	
	// if (use_color)
	// 	return uLight.intensity * (vec3(noise(vST*noise_factor)) * 0.2 + vec3(noise(vST*noise_factor)) * max(dot(ldir,_normal), 0.0) +  specular * pow(max(dot(r,_vViewDir),0), uMaterial.shininess));

	//return uLight.intensity * (uMaterial.ambient + uMaterial.diffuse * max(dot(ldir,_normal), 0.0) +  specular * pow(max(dot(r,_vViewDir),0), uMaterial.shininess));

	return uLight.intensity * (uMaterial.ambient + uMaterial.diffuse * max(dot(ldir,_normal), 0.0) +  specular * pow(max(dot(r,_vViewDir),0), uMaterial.shininess));
}
/*
*
* OLD NOISE FUNCTION
*
*/
// float noise(vec2 st) {
// 	vec2 i = floor(st);
// 	vec2 f = fract(st);

// 	float a = random(i);
// 	float b = random(i + vec2(1.0, 0.0));
// 	float c = random(i + vec2(0.0, 1.0));
// 	float d = random(i + vec2(1.0, 1.0));

// 	vec2 u = f*f*(3.0-2.0*f);

// 	return mix(a,b,u.x) + (c-a) * u.y * (1.0 - u.x) + (d-b) * u.x * u.y;
// }

float noise2 (float x, float y, float z) {
	int X = int(int(floor(x)) & 255),
		Y = int(int(floor(y)) & 255),
		Z = int(int(floor(z)) & 255);

		x -= floor(x);
		y -= floor(y);
		z -= floor(z);

		float u = fade(x), 
		v = fade(y),
		w = fade(z);

		int A = p[X]+Y, AA = p[A]+Z, AB = p[A+1]+Z,
			B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z;

	return lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z   ),  
                                     grad(p[BA  ], x-1, y  , z   )),
                             lerp(u, grad(p[AB  ], x  , y-1, z   ),  
                                     grad(p[BB  ], x-1, y-1, z   ))),
                     lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1 ),  
                                     grad(p[BA+1], x-1, y  , z-1 )), 
                             lerp(u, grad(p[AB+1], x  , y-1, z-1 ),
                                     grad(p[BB+1], x-1, y-1, z-1 ))));
}


float gestiona_ruido(float x, float y, float z) {
	//  float noise_value1 = noise2(x*noise_factor, y*noise_factor, z*noise_factor);
	//  float noise_value2 = noise2(x*noise_factor2, y*noise_factor2, z*noise_factor2);
	//  float noise_value3 = noise2(x*noise_factor3, y*noise_factor3, z*noise_factor3);
	//  float turbulence = 5.75;
	//  float value = sin(x + z + y + noise_value1*turbulence+0.5*noise_value2*turbulence+0.25*noise_value3*turbulence);
	 float value = sin(noise2(x*noise_factor, y*noise_factor, z*noise_factor));
	return value;
}



void main() {
	vec3 normal;
	float scale = 0.04;
	float bias = -0.02;

	// if (use_normal)
	// 	normal = normalize( 2.0 * vec3(noise(vST*noise_factor)) - vec3(1.0) );
	// else
		normal = vec3(0,0,1);
		//normal = vNormal;

	
	float f0 = gestiona_ruido(vPosition.x, vPosition.y, vPosition.z);
	float fx = gestiona_ruido(vPosition.x + epsilon, vPosition.y, vPosition.z);
	float fy = gestiona_ruido(vPosition.x, vPosition.y + epsilon, vPosition.z);
	float fz = gestiona_ruido(vPosition.x, vPosition.y , vPosition.z + epsilon);
	
	if (use_normal) {
		vec3 dF = vec3((fx - f0) / epsilon, (fy - f0) / epsilon, (fz - f0) / epsilon);
		//vec3 new_normal = normalize(2.0 * normalize(normal - dF) - vec3(1.0));
		fFragColor = vec4(ads(normalize(normal - dF) , normalize(vViewDir), normalize(vLightDir)), 1.);
	} else
		fFragColor = vec4(f0, f0, f0, 1);
		//fFragColor = vec4(ads(normal, normalize(vViewDir), normalize(vLightDir)), 1.);
	//

	/*El blanco y negro*/
	// fFragColor = vec4(noise2(vPosition.x*noise_factor,vPosition.y*noise_factor,vPosition.z*noise_factor),
	// 	noise2(vPosition.x*noise_factor,vPosition.y*noise_factor,vPosition.z*noise_factor),
	// 	noise2(vPosition.x*noise_factor,vPosition.y*noise_factor,vPosition.z*noise_factor),
	// 	1);

	
	
	 
}