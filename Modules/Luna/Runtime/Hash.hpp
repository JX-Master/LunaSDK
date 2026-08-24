/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file hash.hpp
* @author JXMaster
* @date 2020/1/9
*/
#pragma once
#include "Base.hpp"
namespace Luna
{
    namespace Impl
    {
        constexpr const u8 crc8_table[256] =
        {
             0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
             157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
             35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
             190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
             70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
             219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
             101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
             248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
             140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
             17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
             175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
             50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
             202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
             87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
             233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
             116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
        };

        constexpr const u16 crc16_table[256] =
        {
            0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
            0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
            0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
            0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
            0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
            0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
            0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
            0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
            0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
            0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
            0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
            0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
            0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
            0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
            0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
            0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
            0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
            0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
            0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
            0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
            0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
            0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
            0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
            0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
            0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
            0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
            0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
            0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
            0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
            0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
            0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
            0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
        };

        constexpr const u32 crc32_table[256] =
        {
            0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
            0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
            0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
            0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
            0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
            0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
            0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
            0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
            0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
            0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
            0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
            0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
            0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
            0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
            0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
            0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
            0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
            0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
            0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
            0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
            0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
            0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
            0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
            0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
            0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
            0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
            0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
            0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
            0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
            0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
            0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
            0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
            0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
            0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
            0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
            0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
            0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
            0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
            0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
            0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
            0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
            0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
            0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
            0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
            0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
            0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
            0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
            0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
            0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
            0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
            0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
            0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
            0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
            0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
            0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
            0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
            0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
            0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
            0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
            0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
            0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
            0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
            0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
            0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
        };

        constexpr const u64 crc64_table[256] =
        {
            0x0000000000000000, 0x3C3B78E888D80FE1, 0x7876F1D111B01FC2, 0x444D893999681023,
            0x750C207570B452A3, 0x4937589DF86C5D42, 0x0D7AD1A461044D61, 0x3141A94CE9DC4280,
            0x6FF9833DB2BCC861, 0x53C2FBD53A64C780, 0x178F72ECA30CD7A3, 0x2BB40A042BD4D842,
            0x1AF5A348C2089AC2, 0x26CEDBA04AD09523, 0x62835299D3B88500, 0x5EB82A715B608AE1,
            0x5A12C5AC36ADFDE5, 0x6629BD44BE75F204, 0x2264347D271DE227, 0x1E5F4C95AFC5EDC6,
            0x2F1EE5D94619AF46, 0x13259D31CEC1A0A7, 0x5768140857A9B084, 0x6B536CE0DF71BF65,
            0x35EB469184113584, 0x09D03E790CC93A65, 0x4D9DB74095A12A46, 0x71A6CFA81D7925A7,
            0x40E766E4F4A56727, 0x7CDC1E0C7C7D68C6, 0x38919735E51578E5, 0x04AAEFDD6DCD7704,
            0x31C4488F3E8F96ED, 0x0DFF3067B657990C, 0x49B2B95E2F3F892F, 0x7589C1B6A7E786CE,
            0x44C868FA4E3BC44E, 0x78F31012C6E3CBAF, 0x3CBE992B5F8BDB8C, 0x0085E1C3D753D46D,
            0x5E3DCBB28C335E8C, 0x6206B35A04EB516D, 0x264B3A639D83414E, 0x1A70428B155B4EAF,
            0x2B31EBC7FC870C2F, 0x170A932F745F03CE, 0x53471A16ED3713ED, 0x6F7C62FE65EF1C0C,
            0x6BD68D2308226B08, 0x57EDF5CB80FA64E9, 0x13A07CF2199274CA, 0x2F9B041A914A7B2B,
            0x1EDAAD56789639AB, 0x22E1D5BEF04E364A, 0x66AC5C8769262669, 0x5A97246FE1FE2988,
            0x042F0E1EBA9EA369, 0x381476F63246AC88, 0x7C59FFCFAB2EBCAB, 0x4062872723F6B34A,
            0x71232E6BCA2AF1CA, 0x4D18568342F2FE2B, 0x0955DFBADB9AEE08, 0x356EA7525342E1E9,
            0x6388911E7D1F2DDA, 0x5FB3E9F6F5C7223B, 0x1BFE60CF6CAF3218, 0x27C51827E4773DF9,
            0x1684B16B0DAB7F79, 0x2ABFC98385737098, 0x6EF240BA1C1B60BB, 0x52C9385294C36F5A,
            0x0C711223CFA3E5BB, 0x304A6ACB477BEA5A, 0x7407E3F2DE13FA79, 0x483C9B1A56CBF598,
            0x797D3256BF17B718, 0x45464ABE37CFB8F9, 0x010BC387AEA7A8DA, 0x3D30BB6F267FA73B,
            0x399A54B24BB2D03F, 0x05A12C5AC36ADFDE, 0x41ECA5635A02CFFD, 0x7DD7DD8BD2DAC01C,
            0x4C9674C73B06829C, 0x70AD0C2FB3DE8D7D, 0x34E085162AB69D5E, 0x08DBFDFEA26E92BF,
            0x5663D78FF90E185E, 0x6A58AF6771D617BF, 0x2E15265EE8BE079C, 0x122E5EB66066087D,
            0x236FF7FA89BA4AFD, 0x1F548F120162451C, 0x5B19062B980A553F, 0x67227EC310D25ADE,
            0x524CD9914390BB37, 0x6E77A179CB48B4D6, 0x2A3A28405220A4F5, 0x160150A8DAF8AB14,
            0x2740F9E43324E994, 0x1B7B810CBBFCE675, 0x5F3608352294F656, 0x630D70DDAA4CF9B7,
            0x3DB55AACF12C7356, 0x018E224479F47CB7, 0x45C3AB7DE09C6C94, 0x79F8D39568446375,
            0x48B97AD9819821F5, 0x7482023109402E14, 0x30CF8B0890283E37, 0x0CF4F3E018F031D6,
            0x085E1C3D753D46D2, 0x346564D5FDE54933, 0x7028EDEC648D5910, 0x4C139504EC5556F1,
            0x7D523C4805891471, 0x416944A08D511B90, 0x0524CD9914390BB3, 0x391FB5719CE10452,
            0x67A79F00C7818EB3, 0x5B9CE7E84F598152, 0x1FD16ED1D6319171, 0x23EA16395EE99E90,
            0x12ABBF75B735DC10, 0x2E90C79D3FEDD3F1, 0x6ADD4EA4A685C3D2, 0x56E6364C2E5DCC33,
            0x42F0E1EBA9EA3693, 0x7ECB990321323972, 0x3A86103AB85A2951, 0x06BD68D2308226B0,
            0x37FCC19ED95E6430, 0x0BC7B97651866BD1, 0x4F8A304FC8EE7BF2, 0x73B148A740367413,
            0x2D0962D61B56FEF2, 0x11321A3E938EF113, 0x557F93070AE6E130, 0x6944EBEF823EEED1,
            0x580542A36BE2AC51, 0x643E3A4BE33AA3B0, 0x2073B3727A52B393, 0x1C48CB9AF28ABC72,
            0x18E224479F47CB76, 0x24D95CAF179FC497, 0x6094D5968EF7D4B4, 0x5CAFAD7E062FDB55,
            0x6DEE0432EFF399D5, 0x51D57CDA672B9634, 0x1598F5E3FE438617, 0x29A38D0B769B89F6,
            0x771BA77A2DFB0317, 0x4B20DF92A5230CF6, 0x0F6D56AB3C4B1CD5, 0x33562E43B4931334,
            0x0217870F5D4F51B4, 0x3E2CFFE7D5975E55, 0x7A6176DE4CFF4E76, 0x465A0E36C4274197,
            0x7334A9649765A07E, 0x4F0FD18C1FBDAF9F, 0x0B4258B586D5BFBC, 0x3779205D0E0DB05D,
            0x06388911E7D1F2DD, 0x3A03F1F96F09FD3C, 0x7E4E78C0F661ED1F, 0x427500287EB9E2FE,
            0x1CCD2A5925D9681F, 0x20F652B1AD0167FE, 0x64BBDB88346977DD, 0x5880A360BCB1783C,
            0x69C10A2C556D3ABC, 0x55FA72C4DDB5355D, 0x11B7FBFD44DD257E, 0x2D8C8315CC052A9F,
            0x29266CC8A1C85D9B, 0x151D14202910527A, 0x51509D19B0784259, 0x6D6BE5F138A04DB8,
            0x5C2A4CBDD17C0F38, 0x6011345559A400D9, 0x245CBD6CC0CC10FA, 0x1867C58448141F1B,
            0x46DFEFF5137495FA, 0x7AE4971D9BAC9A1B, 0x3EA91E2402C48A38, 0x029266CC8A1C85D9,
            0x33D3CF8063C0C759, 0x0FE8B768EB18C8B8, 0x4BA53E517270D89B, 0x779E46B9FAA8D77A,
            0x217870F5D4F51B49, 0x1D43081D5C2D14A8, 0x590E8124C545048B, 0x6535F9CC4D9D0B6A,
            0x54745080A44149EA, 0x684F28682C99460B, 0x2C02A151B5F15628, 0x1039D9B93D2959C9,
            0x4E81F3C86649D328, 0x72BA8B20EE91DCC9, 0x36F7021977F9CCEA, 0x0ACC7AF1FF21C30B,
            0x3B8DD3BD16FD818B, 0x07B6AB559E258E6A, 0x43FB226C074D9E49, 0x7FC05A848F9591A8,
            0x7B6AB559E258E6AC, 0x4751CDB16A80E94D, 0x031C4488F3E8F96E, 0x3F273C607B30F68F,
            0x0E66952C92ECB40F, 0x325DEDC41A34BBEE, 0x761064FD835CABCD, 0x4A2B1C150B84A42C,
            0x1493366450E42ECD, 0x28A84E8CD83C212C, 0x6CE5C7B54154310F, 0x50DEBF5DC98C3EEE,
            0x619F161120507C6E, 0x5DA46EF9A888738F, 0x19E9E7C031E063AC, 0x25D29F28B9386C4D,
            0x10BC387AEA7A8DA4, 0x2C87409262A28245, 0x68CAC9ABFBCA9266, 0x54F1B14373129D87,
            0x65B0180F9ACEDF07, 0x598B60E71216D0E6, 0x1DC6E9DE8B7EC0C5, 0x21FD913603A6CF24,
            0x7F45BB4758C645C5, 0x437EC3AFD01E4A24, 0x07334A9649765A07, 0x3B08327EC1AE55E6,
            0x0A499B3228721766, 0x3672E3DAA0AA1887, 0x723F6AE339C208A4, 0x4E04120BB11A0745,
            0x4AAEFDD6DCD77041, 0x7695853E540F7FA0, 0x32D80C07CD676F83, 0x0EE374EF45BF6062,
            0x3FA2DDA3AC6322E2, 0x0399A54B24BB2D03, 0x47D42C72BDD33D20, 0x7BEF549A350B32C1,
            0x25577EEB6E6BB820, 0x196C0603E6B3B7C1, 0x5D218F3A7FDBA7E2, 0x611AF7D2F703A803,
            0x505B5E9E1EDFEA83, 0x6C6026769607E562, 0x282DAF4F0F6FF541, 0x1416D7A787B7FAA0
        };

        template <typename _HashTy>
        constexpr _HashTy get_crc_table_value(u8 index);

        template <>
        inline constexpr u8 get_crc_table_value<u8>(u8 index)
        {
            return crc8_table[index];
        }

        template <>
        inline constexpr u16 get_crc_table_value<u16>(u8 index)
        {
            return crc16_table[index];
        }

        template <>
        inline constexpr u32 get_crc_table_value<u32>(u8 index)
        {
            return crc32_table[index];
        }

        template <>
        inline constexpr u64 get_crc_table_value<u64>(u8 index)
        {
            return crc64_table[index];
        }
    }

    //! @addtogroup Runtime
    //! @{
    //! @defgroup RuntimeHash Hashing functions
    //! @}

    //! @addtogroup RuntimeHash
    //! @{
    
    //! Computes a hash code for the specified binary data.
    //! @details This is the basic hash function that uses the standard table-driven crc
    //! algorithm (crc-8, crc-16, crc-32 or crc-64, depending on the hash value type) to
    //! hash any kind of binary data stream to a single hash value.
    //! @param[in] data A pointer to the data to be hashed.
    //! @param[in] size The length of the data in bytes.
    //! @param[in] h A initial hash value. If this is a new hash, set to 0 (which
    //! is the default value if not specified). If this is a rehash operation or a second
    //! have before another one, you can specify the last hash value to get a different 
    //! hash value from the same data.
    //! @return Returns the hash code of the data.
    template <typename _HashTy = usize>
    inline _HashTy memhash(const void* data, usize size, _HashTy h = 0)
    {
        const u8* s = reinterpret_cast<const u8*>(data);
        for (; size > 0; --size)
        {
            h = Impl::get_crc_table_value<_HashTy>((h ^ (*s)) & 0xff) ^ (h >> 8);
            ++s;
        }
        return h;
    }

    template <>
    inline u8 memhash<u8>(const void* data, usize size, u8 h)
    {
        const u8* s = reinterpret_cast<const u8*>(data);
        for (; size > 0; --size)
        {
            h = Impl::get_crc_table_value<u8>(h ^ (*s));
            ++s;
        }
        return h;
    }

    //! A specialization of @ref memhash that computes 8-bit hash code.
    //! @param[in] data A pointer to the data to be hashed.
    //! @param[in] size The length of the data in bytes.
    //! @param[in] h A initial hash value. See @ref memhash for details.
    //! @return Returns the hash code of the data.
    inline u8 memhash8(const void* data, usize size, u8 h = 0)
    {
        return memhash<u8>(data, size, h);
    }

    //! A specialization of @ref memhash that computes 16-bit hash code.
    //! @param[in] data A pointer to the data to be hashed.
    //! @param[in] size The length of the data in bytes.
    //! @param[in] h A initial hash value. See @ref memhash for details.
    //! @return Returns the hash code of the data.
    inline u16 memhash16(const void* data, usize size, u16 h = 0)
    {
        return memhash<u16>(data, size, h);
    }

    //! A specialization of @ref memhash that computes 32-bit hash code.
    //! @param[in] data A pointer to the data to be hashed.
    //! @param[in] size The length of the data in bytes.
    //! @param[in] h A initial hash value. See @ref memhash for details.
    //! @return Returns the hash code of the data.
    inline u32 memhash32(const void* data, usize size, u32 h = 0)
    {
        return memhash<u32>(data, size, h);
    }

    //! A specialization of @ref memhash that computes 64-bit hash code.
    //! @param[in] data A pointer to the data to be hashed.
    //! @param[in] size The length of the data in bytes.
    //! @param[in] h A initial hash value. See @ref memhash for details.
    //! @return Returns the hash code of the data.
    inline u64 memhash64(const void* data, usize size, u64 h = 0)
    {
        return memhash<u64>(data, size, h);
    }

    //! Computes a hash code for the specified string.
    //! @param[in] s A pointer to one null-terminated string to compute.
    //! @param[in] h A initial hash value. See @ref memhash for details.
    //! @return Returns the hash code of the string.
    template <typename _HashTy = usize>
    inline constexpr _HashTy strhash(const c8* s, _HashTy h = 0)
    {
        while (*s)
        {
            h = Impl::get_crc_table_value<_HashTy>((h ^ (*s)) & 0xff) ^ (h >> 8);
            ++s;
        }
        return h;
    }

    template <>
    inline constexpr u8 strhash<u8>(const c8* s, u8 h)
    {
        while (*s)
        {
            h = Impl::get_crc_table_value<u8>(h ^ (*s));
            ++s;
        }
        return h;
    }

    //! A specialization of @ref strhash that computes 8-bit hash code.
    //! @param[in] s A pointer to one null-terminated string to compute.
    //! @param[in] h A initial hash value. See @ref memhash for details.
    //! @return Returns the hash code of the string.
    inline constexpr u8 strhash8(const c8* s, u8 h = 0)
    {
        return strhash<u8>(s, h);
    }

    //! A specialization of @ref strhash that computes 16-bit hash code.
    //! @param[in] s A pointer to one null-terminated string to compute.
    //! @param[in] h A initial hash value. See @ref memhash for details.
    //! @return Returns the hash code of the string.
    inline constexpr u16 strhash16(const c8* s, u16 h = 0)
    {
        return strhash<u16>(s, h);
    }

    //! A specialization of @ref strhash that computes 32-bit hash code.
    //! @param[in] s A pointer to one null-terminated string to compute.
    //! @param[in] h A initial hash value. See @ref memhash for details.
    //! @return Returns the hash code of the string.
    inline constexpr u32 strhash32(const c8* s, u32 h = 0)
    {
        return strhash<u32>(s, h);
    }

    //! A specialization of @ref strhash that computes 64-bit hash code.
    //! @param[in] s A pointer to one null-terminated string to compute.
    //! @param[in] h A initial hash value. See @ref memhash for details.
    //! @return Returns the hash code of the string.
    inline constexpr u64 strhash64(const c8* s, u64 h = 0)
    {
        return strhash<u64>(s, h);
    }

    //! @}
}
