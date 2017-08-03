// This is an automatically generated file.
// Generated from this ImEmgData.msg definition:
//   float64[] imEmgData
//   float64[] filteredImEmgData
//   float64[] normImEmgData
//   float64[] idxCC// Instances of this class can be read and written with YARP ports,
// using a ROS-compatible format.

#ifndef YARPMSG_TYPE_ImEmgData
#define YARPMSG_TYPE_ImEmgData

#include <string>
#include <vector>
#include <yarp/os/Wire.h>
#include <yarp/os/idl/WireTypes.h>

class ImEmgData : public yarp::os::idl::WirePortable {
public:
  std::vector<yarp::os::NetFloat64> imEmgData;
  std::vector<yarp::os::NetFloat64> filteredImEmgData;
  std::vector<yarp::os::NetFloat64> normImEmgData;
  std::vector<yarp::os::NetFloat64> idxCC;

  ImEmgData() {
  }

  void clear() {
    // *** imEmgData ***
    imEmgData.clear();

    // *** filteredImEmgData ***
    filteredImEmgData.clear();

    // *** normImEmgData ***
    normImEmgData.clear();

    // *** idxCC ***
    idxCC.clear();
  }

  bool readBare(yarp::os::ConnectionReader& connection) YARP_OVERRIDE {
    // *** imEmgData ***
    int len = connection.expectInt();
    imEmgData.resize(len);
    if (len > 0 && !connection.expectBlock((char*)&imEmgData[0],sizeof(yarp::os::NetFloat64)*len)) return false;

    // *** filteredImEmgData ***
    len = connection.expectInt();
    filteredImEmgData.resize(len);
    if (len > 0 && !connection.expectBlock((char*)&filteredImEmgData[0],sizeof(yarp::os::NetFloat64)*len)) return false;

    // *** normImEmgData ***
    len = connection.expectInt();
    normImEmgData.resize(len);
    if (len > 0 && !connection.expectBlock((char*)&normImEmgData[0],sizeof(yarp::os::NetFloat64)*len)) return false;

    // *** idxCC ***
    len = connection.expectInt();
    idxCC.resize(len);
    if (len > 0 && !connection.expectBlock((char*)&idxCC[0],sizeof(yarp::os::NetFloat64)*len)) return false;
    return !connection.isError();
  }

  bool readBottle(yarp::os::ConnectionReader& connection) YARP_OVERRIDE {
    connection.convertTextMode();
    yarp::os::idl::WireReader reader(connection);
    if (!reader.readListHeader(4)) return false;

    // *** imEmgData ***
    if (connection.expectInt()!=(BOTTLE_TAG_LIST|BOTTLE_TAG_DOUBLE)) return false;
    int len = connection.expectInt();
    imEmgData.resize(len);
    for (int i=0; i<len; i++) {
      imEmgData[i] = (yarp::os::NetFloat64)connection.expectDouble();
    }

    // *** filteredImEmgData ***
    if (connection.expectInt()!=(BOTTLE_TAG_LIST|BOTTLE_TAG_DOUBLE)) return false;
    len = connection.expectInt();
    filteredImEmgData.resize(len);
    for (int i=0; i<len; i++) {
      filteredImEmgData[i] = (yarp::os::NetFloat64)connection.expectDouble();
    }

    // *** normImEmgData ***
    if (connection.expectInt()!=(BOTTLE_TAG_LIST|BOTTLE_TAG_DOUBLE)) return false;
    len = connection.expectInt();
    normImEmgData.resize(len);
    for (int i=0; i<len; i++) {
      normImEmgData[i] = (yarp::os::NetFloat64)connection.expectDouble();
    }

    // *** idxCC ***
    if (connection.expectInt()!=(BOTTLE_TAG_LIST|BOTTLE_TAG_DOUBLE)) return false;
    len = connection.expectInt();
    idxCC.resize(len);
    for (int i=0; i<len; i++) {
      idxCC[i] = (yarp::os::NetFloat64)connection.expectDouble();
    }
    return !connection.isError();
  }

  using yarp::os::idl::WirePortable::read;
  bool read(yarp::os::ConnectionReader& connection) YARP_OVERRIDE {
    if (connection.isBareMode()) return readBare(connection);
    return readBottle(connection);
  }

  bool writeBare(yarp::os::ConnectionWriter& connection) YARP_OVERRIDE {
    // *** imEmgData ***
    connection.appendInt(imEmgData.size());
    if (imEmgData.size()>0) {connection.appendExternalBlock((char*)&imEmgData[0],sizeof(yarp::os::NetFloat64)*imEmgData.size());}

    // *** filteredImEmgData ***
    connection.appendInt(filteredImEmgData.size());
    if (filteredImEmgData.size()>0) {connection.appendExternalBlock((char*)&filteredImEmgData[0],sizeof(yarp::os::NetFloat64)*filteredImEmgData.size());}

    // *** normImEmgData ***
    connection.appendInt(normImEmgData.size());
    if (normImEmgData.size()>0) {connection.appendExternalBlock((char*)&normImEmgData[0],sizeof(yarp::os::NetFloat64)*normImEmgData.size());}

    // *** idxCC ***
    connection.appendInt(idxCC.size());
    if (idxCC.size()>0) {connection.appendExternalBlock((char*)&idxCC[0],sizeof(yarp::os::NetFloat64)*idxCC.size());}
    return !connection.isError();
  }

  bool writeBottle(yarp::os::ConnectionWriter& connection) YARP_OVERRIDE {
    connection.appendInt(BOTTLE_TAG_LIST);
    connection.appendInt(4);

    // *** imEmgData ***
    connection.appendInt(BOTTLE_TAG_LIST|BOTTLE_TAG_DOUBLE);
    connection.appendInt(imEmgData.size());
    for (size_t i=0; i<imEmgData.size(); i++) {
      connection.appendDouble((double)imEmgData[i]);
    }

    // *** filteredImEmgData ***
    connection.appendInt(BOTTLE_TAG_LIST|BOTTLE_TAG_DOUBLE);
    connection.appendInt(filteredImEmgData.size());
    for (size_t i=0; i<filteredImEmgData.size(); i++) {
      connection.appendDouble((double)filteredImEmgData[i]);
    }

    // *** normImEmgData ***
    connection.appendInt(BOTTLE_TAG_LIST|BOTTLE_TAG_DOUBLE);
    connection.appendInt(normImEmgData.size());
    for (size_t i=0; i<normImEmgData.size(); i++) {
      connection.appendDouble((double)normImEmgData[i]);
    }

    // *** idxCC ***
    connection.appendInt(BOTTLE_TAG_LIST|BOTTLE_TAG_DOUBLE);
    connection.appendInt(idxCC.size());
    for (size_t i=0; i<idxCC.size(); i++) {
      connection.appendDouble((double)idxCC[i]);
    }
    connection.convertTextMode();
    return !connection.isError();
  }

  using yarp::os::idl::WirePortable::write;
  bool write(yarp::os::ConnectionWriter& connection) YARP_OVERRIDE {
    if (connection.isBareMode()) return writeBare(connection);
    return writeBottle(connection);
  }

  // This class will serialize ROS style or YARP style depending on protocol.
  // If you need to force a serialization style, use one of these classes:
  typedef yarp::os::idl::BareStyle<ImEmgData> rosStyle;
  typedef yarp::os::idl::BottleStyle<ImEmgData> bottleStyle;

  // Give source text for class, ROS will need this
  yarp::os::ConstString getTypeText() {
    return "float64[] imEmgData\n\
float64[] filteredImEmgData\n\
float64[] normImEmgData\n\
float64[] idxCC";
  }

  // Name the class, ROS will need this
  yarp::os::Type getType() YARP_OVERRIDE {
    yarp::os::Type typ = yarp::os::Type::byName("ImEmgData","ImEmgData");
    typ.addProperty("md5sum",yarp::os::Value("30610af7259271b4c9e83f914423bb3a"));
    typ.addProperty("message_definition",yarp::os::Value(getTypeText()));
    return typ;
  }
};

#endif
