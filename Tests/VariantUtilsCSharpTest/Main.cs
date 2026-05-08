using System;
using Luna.Runtime;
using Luna.VariantUtils;

Runtime.Init();

try
{
    VariantUtilsModule.Init();

    using (var jsonVariant = VariantUtilsModule.ReadJson("""
    {
        "status": "0000",
        "message": "success",
        "response": true,
        "data": {
            "content": [
                { "id": 1, "value": "37.0" },
                { "id": 2, "value": "72.3" }
            ],
            "meta": null
        }
    }
    """))
    {
        var jsonText = VariantUtilsModule.WriteJson(jsonVariant);
        using var reparsedJson = VariantUtilsModule.ReadJson(jsonText);
        if (jsonVariant != reparsedJson)
        {
            throw new InvalidOperationException("JSON roundtrip failed.");
        }
    }

    var blobSource = System.Text.Encoding.UTF8.GetBytes("Sample BLOB Data");
    using (var blobVariant = new Variant(blobSource))
    {
        var json = VariantUtilsModule.WriteJson(blobVariant);
        using var parsedBlob = VariantUtilsModule.ReadJson(json);
        if (blobVariant != parsedBlob)
        {
            throw new InvalidOperationException("JSON blob roundtrip failed.");
        }
    }

    using (var negativeNumber = new Variant(-3L))
    {
        var json = VariantUtilsModule.WriteJson(negativeNumber);
        using var reparsed = VariantUtilsModule.ReadJson(json);
        if (negativeNumber != reparsed)
        {
            throw new InvalidOperationException("Negative number JSON roundtrip failed.");
        }
    }

    using (var xmlRoundtripVariant = VariantUtilsModule.ReadXml("""
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
    """))
    {
        var xmlText = VariantUtilsModule.WriteXml(xmlRoundtripVariant);
        using var reparsedXml = VariantUtilsModule.ReadXml(xmlText);
        if (xmlRoundtripVariant != reparsedXml)
        {
            var beforeJson = VariantUtilsModule.WriteJson(xmlRoundtripVariant);
            var afterJson = VariantUtilsModule.WriteJson(reparsedXml);
            throw new InvalidOperationException($"XML roundtrip failed.\nXML:\n{xmlText}\nBefore JSON:\n{beforeJson}\nAfter JSON:\n{afterJson}");
        }
    }

    using (var xmlVariant = VariantUtilsModule.ReadXml("""
    <?xml version="1.0" encoding="UTF-8"?>
    <p>This is a <a>hinted</a> paragraph.</p>
    """))
    {
        using var content = VariantUtilsModule.GetXmlContent(xmlVariant);
        using var prefix = content.GetArrayItem(0);
        using var suffix = content.GetArrayItem(2);
        if (prefix.GetString() != "This is a " || suffix.GetString() != " paragraph.")
        {
            throw new InvalidOperationException("XML mixed content parsing failed.");
        }
    }

    using (var before = VariantUtilsModule.ReadJson("""{ "p" : false, "nested": { "value": 1 } }"""))
    using (var after = VariantUtilsModule.ReadJson("""{ "p" : true, "nested": { "value": 2 }, "extra": [1,2,3] }"""))
    using (var delta = VariantUtilsModule.Diff(before, after))
    {
        using var patched = before.Clone();
        VariantUtilsModule.Patch(patched, delta);
        if (patched != after)
        {
            throw new InvalidOperationException("Variant patch failed.");
        }

        VariantUtilsModule.Revert(patched, delta);
        if (patched != before)
        {
            throw new InvalidOperationException("Variant revert failed.");
        }
    }

    using (var root = VariantUtilsModule.NewXmlElement("root"))
    {
        if (VariantUtilsModule.GetXmlName(root) != "root")
        {
            throw new InvalidOperationException("NewXmlElement/GetXmlName failed.");
        }
        VariantUtilsModule.SetXmlName(root, "renamed");
        if (VariantUtilsModule.GetXmlName(root) != "renamed")
        {
            throw new InvalidOperationException("SetXmlName failed.");
        }
    }

    Console.WriteLine("VariantUtilsCSharpTest passed.");
}
catch (Exception ex)
{
    Console.Error.WriteLine(ex);
    throw;
}
finally
{
    Runtime.Close();
}
