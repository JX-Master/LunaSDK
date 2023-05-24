RWStructuredBuffer<uint> g_dest_buffer : register(u0);
[numthreads(256, 1, 1)]
void main(uint group_index : SV_GroupIndex)
{
    g_dest_buffer[group_index] = 0;
}