#version 460
#extension GL_EXT_ray_query : require
#extension GL_EXT_ray_tracing_position_fetch : require

layout(location = 0) out vec3 o_color;
layout(set = 0, binding = 0) uniform accelerationStructureEXT uAS;

void main()
{
	rayQueryEXT query;
	rayQueryInitializeEXT(query, uAS, gl_RayFlagsNoOpaqueEXT, 0xff,
	                      vec3(0.0), 0.01, vec3(1.0, 0.0, 0.0), 1000.0);
	vec3 sum = vec3(0.0);
	while (rayQueryProceedEXT(query))
	{
		if (rayQueryGetIntersectionTypeEXT(query, false) ==
		    gl_RayQueryCandidateIntersectionTriangleEXT)
		{
			vec3 candidate[3];
			rayQueryGetIntersectionTriangleVertexPositionsEXT(query, false, candidate);
			sum += candidate[0] + candidate[1] + candidate[2];
			rayQueryConfirmIntersectionEXT(query);
		}
	}
	if (rayQueryGetIntersectionTypeEXT(query, true) ==
	    gl_RayQueryCommittedIntersectionTriangleEXT)
	{
		vec3 committed[3];
		rayQueryGetIntersectionTriangleVertexPositionsEXT(query, true, committed);
		sum += committed[0] + committed[1] + committed[2];
	}
	o_color = sum;
}
