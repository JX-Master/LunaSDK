/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file XMLTest.cpp
* @author JXMaster
* @date 2021/8/3
*/
#include "TestCommon.hpp"
#include <Luna/VariantUtils/XML.hpp>
#include <Luna/Runtime/Log.hpp>

namespace Luna
{
    void xml_test()
    {
        {
            const c8* src = R"(
<?xml version="1.0" encoding="UTF-8"?>
<bookstore>
    <book category="COOKING">
        <title lang="en">Everyday Italian</title>
        <author>Giada De Laurentiis</author>
        <year>2005</year>
        <price>30.00</price>
    </book>
    <book category="CHILDREN">
        <title lang="en">Harry Potter</title>
        <author>J K. Rowling</author>
        <year>2005</year>
        <price>29.99</price>
    </book>
    <book category="WEB">
        <title lang="en">Learning XML</title>
        <author>Erik T. Ray</author>
        <year>2003</year>
        <price>39.95</price>
        <display><![CDATA[<p>Learning XML</p>]]></display>
    </book>
</bookstore>
        )";
            R<Variant> v = VariantUtils::read_xml(src);
            if (failed(v)) log_error("xml_test", "%s", explain(v.errcode()));
            luassert_always(succeeded(v));
            String s = VariantUtils::write_xml(v.get());
            log_debug("xml_test", "%s", s.c_str());
            R<Variant> v2 = VariantUtils::read_xml(s.c_str());
            if (failed(v2)) log_error("xml_test", "%s", explain(v2.errcode()));
            luassert_always(succeeded(v2));
            luassert_always(v.get() == v2.get());
        }
        {
            const c8* src = R"(
<?xml version="1.0" encoding="UTF-8"?>
<p>This is a <a>hinted</a> paragraph.</p>
            )";
            R<Variant> v = VariantUtils::read_xml(src);
            if (failed(v)) log_error("xml_test", "%s", explain(v.errcode()));
            luassert_always(succeeded(v));
            auto& elements = VariantUtils::get_xml_content(v.get());
            luassert_always(elements.at(0).str() == "This is a ");
            luassert_always(elements.at(2).str() == " paragraph.");
        }
    }
}