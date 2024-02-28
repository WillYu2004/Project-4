#include <gtest/gtest.h>
#include "OpenStreetMap.h"
#include "StreetMap.h"
#include "StringDataSource.h"
#include "XMLReader.h"
TEST(OpenStreetMap, SimpleExampleTest){
    auto InStream = std::make_shared<CStringDataSource>("<?xml version='1.0' encoding='UTF-8'?>"
                                                        "<osm version=\"0.6\" generator=\"osmconvert 0.8.5\">"
                                                        "<node id=\"1\" lat=\"1.0\" lon=\"-1.0\"/>"
                                                        "</osm>");
    auto Reader = std::make_shared<CXMLReader>(InStream);
    COpenStreetMap StreetMap(Reader);
    
    EXPECT_EQ(StreetMap.NodeCount(),1);
    EXPECT_EQ(StreetMap.WayCount(),0);
    EXPECT_EQ(StreetMap.NodeByIndex(0)->ID(),1);
}
TEST(OpenStreetMap, MultipleNodesTest) {
    auto InStream = std::make_shared<CStringDataSource>(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm>"
        "<node id=\"1\" lat=\"1.0\" lon=\"-1.0\"/>"
        "<node id=\"2\" lat=\"2.0\" lon=\"-2.0\"/>"
        "</osm>");
    auto Reader = std::make_shared<CXMLReader>(InStream);
    COpenStreetMap StreetMap(Reader);
    
    EXPECT_EQ(StreetMap.NodeCount(), 2);
    EXPECT_EQ(StreetMap.NodeByIndex(1)->ID(), 2);
}
TEST(OpenStreetMap, WayParsingTest) {
    auto InStream = std::make_shared<CStringDataSource>(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm>"
        "<node id=\"1\" lat=\"1.0\" lon=\"-1.0\"/>"
        "<node id=\"2\" lat=\"2.0\" lon=\"-2.0\"/>"
        "<way id=\"10\">"
        "<nd ref=\"1\"/>"
        "<nd ref=\"2\"/>"
        "</way>"
        "</osm>");
    auto Reader = std::make_shared<CXMLReader>(InStream);
    COpenStreetMap StreetMap(Reader);
    
    EXPECT_EQ(StreetMap.WayCount(), 1);
    auto way = StreetMap.WayByIndex(0);
    EXPECT_NE(way, nullptr);
    EXPECT_EQ(way->NodeCount(), 2);
    EXPECT_EQ(way->GetNodeID(0), 1);
    EXPECT_EQ(way->GetNodeID(1), 2);
}
TEST(OpenStreetMap, NodeByIDValidTest) {
    // Set up XML data source with nodes
    auto InStream = std::make_shared<CStringDataSource>(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm><node id=\"100\" lat=\"10.0\" lon=\"20.0\"/></osm>");
    auto Reader = std::make_shared<CXMLReader>(InStream);
    COpenStreetMap Map(Reader);
    
    // Check that the node with ID 100 can be retrieved and has the correct location
    auto node = Map.NodeByID(100);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->ID(), 100);
    EXPECT_EQ(node->Location().first, 10.0);
    EXPECT_EQ(node->Location().second, 20.0);
}
TEST(OpenStreetMap, NodeByIDInvalidTest) {
    // Set up XML data source without nodes for simplicity
    auto InStream = std::make_shared<CStringDataSource>("<?xml version='1.0' encoding='UTF-8'?><osm></osm>");
    auto Reader = std::make_shared<CXMLReader>(InStream);
    COpenStreetMap Map(Reader);
    
    // Check that an invalid ID returns nullptr
    auto node = Map.NodeByID(999); // Assuming 999 does not exist
    EXPECT_EQ(node, nullptr);
}
TEST(OpenStreetMap, WayByIDValidTest) {
    // Set up XML data source with one way
    auto InStream = std::make_shared<CStringDataSource>(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm><way id=\"200\"><nd ref=\"100\"/><tag k=\"highway\" v=\"residential\"/></way></osm>");
    auto Reader = std::make_shared<CXMLReader>(InStream);
    COpenStreetMap Map(Reader);
    
    // Check that the way with ID 200 can be retrieved and has the correct properties
    auto way = Map.WayByID(200);
    ASSERT_NE(way, nullptr);
    EXPECT_EQ(way->ID(), 200);
    EXPECT_EQ(way->NodeCount(), 1); // Assumes the node with ref 100 exists
    EXPECT_TRUE(way->HasAttribute("highway"));
}
TEST(OpenStreetMap, WayByIDInvalidTest) {
    // Set up XML data source without ways for simplicity
    auto InStream = std::make_shared<CStringDataSource>("<?xml version='1.0' encoding='UTF-8'?><osm></osm>");
    auto Reader = std::make_shared<CXMLReader>(InStream);
    COpenStreetMap Map(Reader);
    
    // Check that an invalid ID returns nullptr
    auto way = Map.WayByID(300); // Assuming 300 does not exist
    EXPECT_EQ(way, nullptr);
}
TEST(OpenStreetMap, GetAttributeKeyTest) {
    auto InStream = std::make_shared<CStringDataSource>(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm>"
        "<node id=\"1\" lat=\"50.0\" lon=\"20.0\">"
        "<tag k=\"name\" v=\"Test Node\"/>"
        "<tag k=\"amenity\" v=\"cafe\"/>"
        "</node>"
        "</osm>");
    auto Reader = std::make_shared<CXMLReader>(InStream);
    COpenStreetMap Map(Reader);

    // Assuming NodeByIndex(0) successfully retrieves the first node
    auto node = Map.NodeByIndex(0);
    ASSERT_NE(node, nullptr); // Ensure the node is not null

    // Verify the attribute keys are correctly retrieved
    EXPECT_EQ(node->GetAttributeKey(0), "name");
    EXPECT_EQ(node->GetAttributeKey(1), "amenity");

    // Verify that an out-of-range index returns an empty string
    EXPECT_TRUE(node->GetAttributeKey(2).empty());
}
TEST(OpenStreetMap, EmptyXMLTest) {
    auto InStream = std::make_shared<CStringDataSource>("<?xml version='1.0' encoding='UTF-8'?><osm></osm>");
    auto Reader = std::make_shared<CXMLReader>(InStream);
    COpenStreetMap StreetMap(Reader);

    EXPECT_EQ(StreetMap.NodeCount(), 0);
    EXPECT_EQ(StreetMap.WayCount(), 0);
}
TEST(OpenStreetMap, NonExistentAttributeTest) {
    auto InStream = std::make_shared<CStringDataSource>(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm><node id=\"1\" lat=\"1.0\" lon=\"-1.0\">"
        "<tag k=\"amenity\" v=\"cafe\"/></node></osm>");
    auto Reader = std::make_shared<CXMLReader>(InStream);
    COpenStreetMap StreetMap(Reader);
    
    auto node = StreetMap.NodeByID(1);
    EXPECT_FALSE(node->HasAttribute("nonexistent"));
    EXPECT_EQ(node->GetAttribute("nonexistent"), "");
}
TEST(OpenStreetMap, WayWithNoNodesTest) {
    auto InStream = std::make_shared<CStringDataSource>(
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<osm><way id=\"10\"></way></osm>");
    auto Reader = std::make_shared<CXMLReader>(InStream);
    COpenStreetMap StreetMap(Reader);
    
    EXPECT_EQ(StreetMap.WayCount(), 1);
    auto way = StreetMap.WayByIndex(0);
    EXPECT_EQ(way->NodeCount(), 0);
}