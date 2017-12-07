static float4 gl_Position;
static int gl_VertexID;
static int gl_InstanceID;
struct SPIRV_Cross_Input
{
    uint gl_VertexID : SV_VertexID;
    uint gl_InstanceID : SV_InstanceID;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    float _23 = float(gl_VertexID + gl_InstanceID);
    gl_Position = float4(_23, _23, _23, _23);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_VertexID = int(stage_input.gl_VertexID);
    gl_InstanceID = int(stage_input.gl_InstanceID);
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
