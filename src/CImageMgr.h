#ifndef CIMAGE_MGR_H
#define CIMAGE_MGR_H

#define CImageMgrInst CImageMgr::getInstance()

#include <CFileType.h>
#include <CFile.h>
#include <CISize2D.h>
#include <CStrUtil.h>
#include <CImagePtr.h>
#include <map>
#include <list>

class CImage;
class CImageFile;
class CImageSizedFile;
class CImageFmt;

class CImageSrc {
 public:
  enum Type {
    NO_SRC,
    SIZED_FILE_SRC,
    FILE_SRC,
    DATA_SRC,
    NAME_SRC,
    XPM_SRC,
    XBM_SRC
  };

 private:
  Type type_;

 protected:
  CImageSrc(Type type) :
   type_(type) {
  }

 public:
  virtual ~CImageSrc() { }

  bool isType(Type type) const { return (type_ == type); }

  virtual std::string getName() const = 0;
};

class CImageNoSrc : public CImageSrc {
 public:
  CImageNoSrc() :
   CImageSrc(NO_SRC) {
  }

  std::string getName() const { return ""; }
};

class CImageSizedFileSrc : public CImageSrc {
 private:
  std::string filename_;
  int         width_;
  int         height_;
  int         keep_aspect_;

 public:
  CImageSizedFileSrc(const std::string &filename, int width, int height, bool keep_aspect) :
   CImageSrc(SIZED_FILE_SRC), filename_(filename),
   width_(width), height_(height), keep_aspect_(keep_aspect) {
  }

  CImageSizedFileSrc(const CFile &file, int width, int height, bool keep_aspect) :
   CImageSrc(SIZED_FILE_SRC), filename_(file.getPath()),
   width_(width), height_(height), keep_aspect_(keep_aspect) {
  }

  std::string getName() const {
    return "sized_file:" + getFilename() +
           "?width="  + CStrUtil::toString(width_) +
           "&height=" + CStrUtil::toString(height_) +
           "&keep_aspect=" + (keep_aspect_ ? "true" : "false");
  }

  const std::string &getFilename() const { return filename_; }

  int getWidth     () const { return width_ ; }
  int getHeight    () const { return height_; }
  int getKeepAspect() const { return keep_aspect_; }
};

class CImageFileSrc : public CImageSrc {
 private:
  std::string filename_;

 public:
  CImageFileSrc(const std::string &filename="") :
   CImageSrc(FILE_SRC), filename_(filename) {
  }

  CImageFileSrc(const CFile &file) :
   CImageSrc(FILE_SRC), filename_(file.getPath()) {
  }

  const std::string &getFilename() const { return filename_; }

  std::string getName() const { return "file:" + getFilename(); }
};

class CImageDataSrc : public CImageSrc {
 private:
  const std::string data_;
  int               id_;

 public:
  static uint getId() {
    static uint id = 1;

    return id++;
  }

  CImageDataSrc(const std::string &data) :
   CImageSrc(DATA_SRC), data_(data), id_(0) {
    id_ = getId();
  }

  CImageDataSrc(const uchar *data, uint len) :
   CImageSrc(DATA_SRC), data_((const char *) data, len), id_(0) {
    id_ = getId();
  }

  const std::string &getData() const { return data_; }

  const char *getDataP() const { return data_.c_str(); }

  uint getDataLen() const { return data_.size(); }

  std::string getName() const { return "data:" + CStrUtil::toString(id_); }
};

class CImageNameSrc : public CImageSrc {
 private:
  const std::string name_;

 public:
  CImageNameSrc(const std::string &name) :
   CImageSrc(NAME_SRC), name_(name) {
  }

  std::string getName() const { return "name:" + name_; }
};

class CImageXPMSrc : public CImageSrc {
 private:
  const char **strs_;
  uint         num_strs_;
  int          id_;

 public:
  static uint getId() {
    static uint id = 1;

    return id++;
  }

  CImageXPMSrc(const char **strs, uint num_strs) :
   CImageSrc(XPM_SRC), strs_(strs), num_strs_(num_strs), id_(0) {
    id_ = getId();
  }

  CImageXPMSrc(const CImageXPMSrc &rhs) :
   CImageSrc(XPM_SRC), strs_(rhs.strs_), num_strs_(rhs.num_strs_), id_(0) {
   id_ = getId();
  }

  const char **getStrs   () const { return strs_; }
  uint         getNumStrs() const { return num_strs_; }

  std::string getName() const { return "xpm:" + CStrUtil::toString(id_); }

 private:
  CImageXPMSrc &operator=(const CImageXPMSrc &rhs);
};

class CImageXBMSrc : public CImageSrc {
 private:
  uchar *data_;
  int    width_;
  int    height_;
  int    id_;

 public:
  static uint getId() {
    static uint id = 1;

    return id++;
  }

  CImageXBMSrc(uchar *data, int width, int height) :
   CImageSrc(XBM_SRC), data_(data), width_(width), height_(height), id_(0) {
    id_ = getId();
  }

  const uchar *getData  () const { return data_  ; }
  int          getWidth () const { return width_ ; }
  int          getHeight() const { return height_; }

  std::string getName() const { return "xbm:" + CStrUtil::toString(id_); }

 private:
  CImageXBMSrc(const CImageXBMSrc &rhs);
  CImageXBMSrc &operator=(const CImageXBMSrc &rhs);
};

class CImageMgr {
 private:
  typedef std::list<CImage *>                     ImageList;
  typedef std::map<std::string,CImageFile *>      ImageFileMap;
  typedef std::map<std::string,CImageSizedFile *> ImageSizedFileMap;
  typedef std::map<CFileType,CImageFmt *>         TypeFmtMap;

  ImageList         image_list_;
  ImageFileMap      image_file_map_;
  ImageSizedFileMap image_sized_file_map_;
  TypeFmtMap        fmt_map_;
  CImagePtr         prototype_;
  bool              creating_;
  bool              debug_;

  friend class CImage;
  friend class CImageFile;

 public:
  static CImageMgr *getInstance() {
    static CImageMgr *mgr;

    if (mgr == NULL)
      mgr = new CImageMgr;

    return mgr;
  }

  CImageMgr();

 ~CImageMgr();

  bool addFmt(CFileType type, CImageFmt *fmt);
  bool getFmt(CFileType type, CImageFmt **fmt);

 public:
  void setPrototype(CImagePtr ptr) { prototype_ = ptr; }

  CImagePtr getPrototype() const { return prototype_; }

  CImagePtr lookupImage(const std::string &fileName);

  CImagePtr lookupImage(const CImageSrc &src);
  CImagePtr createImage(const CImageSrc &src=CImageNoSrc());

  CImageFile *lookupFile(const CImageSrc &src);

 private:
  CImagePtr createImageI();
  CImagePtr createImageI(const CISize2D &size);
  CImagePtr createImageI(int width, int height);
  CImagePtr createImageI(const CImage &image);
  CImagePtr createImageI(const CImage &image, int x, int y, int width, int height);

  CImageFile *lookupFile(const std::string &fileName);

  CImageFile *addFile(const std::string &fileName);

  bool removeFile(const std::string &fileName);

  CImageSizedFile *lookupSizedFile(const std::string &fileName, int width, int height,
                                   bool keep_aspect=false);

  CImageSizedFile *addSizedFile(const std::string &fileName, int width, int height,
                                bool keep_aspect=false);

  bool removeSizedFile(const std::string &fileName, int width, int height,
                       bool keep_aspect=false);

 protected:
  bool addImage(CImage *);
  bool deleteImage(CImage *);

 private:
  CImageFile *lookupFileI(const std::string &fileName);
  CImageFile *addFileI   (const std::string &fileName);

  CImageSizedFile *lookupSizedFileI(const std::string &fileName, int width, int height,
                                    bool keep_aspect);

  CImageSizedFile *addSizedFileI(const std::string &fileName, int width, int height,
                                 bool keep_aspect);

  CImagePtr newImage();
};

class CImageThumbnailMgr {
 private:
  CISize2D  size_;
  CImageMgr image_mgr_;

 public:
  CImageThumbnailMgr(uint width, uint height);
 ~CImageThumbnailMgr();

  CImagePtr lookupImage(const std::string &name);
};

#endif
