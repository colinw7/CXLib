#ifndef CCONFIG_H
#define CCONFIG_H

#include <CAutoPtr.h>

#include <string>
#include <vector>
#include <list>

class CConfigFile;
class CConfigGroup;
class CConfigSection;

class CConfig {
  typedef std::list<CConfigGroup *> CConfigGroupList;

 private:
  std::string      name_;
  std::string      path_;
  CConfigGroupList groups_;

 public:
  CConfig(const std::string &name);
 ~CConfig();

  void save();

  bool getValue(const std::string &path, std::string &value);
  bool getValue(const std::string &path, bool *value);
  bool getValue(const std::string &path, int *value);
  bool getValue(const std::string &path, double *value);
  bool getValue(const std::string &path, std::vector<std::string> &values);

  bool getValue(const std::string &path, const std::string &sectionName, std::string &value);
  bool getValue(const std::string &path, const std::string &sectionName, bool *value);
  bool getValue(const std::string &path, const std::string &sectionName, int *value);
  bool getValue(const std::string &path, const std::string &sectionName, double *value);
  bool getValue(const std::string &path, const std::string &sectionName,
                std::vector<std::string> &values);

  bool setValue(const std::string &path, const std::string &value);
  bool setValue(const std::string &path, bool value);
  bool setValue(const std::string &path, int value);
  bool setValue(const std::string &path, double value);
  bool setValue(const std::string &path, const std::vector<std::string> &values);

  bool setValue(const std::string &path, const std::string &sectionName, const std::string &value);
  bool setValue(const std::string &path, const std::string &sectionName, bool value);
  bool setValue(const std::string &path, const std::string &sectionName, int value);
  bool setValue(const std::string &path, const std::string &sectionName, double value);
  bool setValue(const std::string &path, const std::string &sectionName,
                const std::vector<std::string> &values);

  bool getValue(const std::string &groupPath, const std::string &sectionName,
                const std::string &name, std::string &value);
  bool setValue(const std::string &groupPath, const std::string &sectionName,
                const std::string &name, const std::string &value);

  bool getGroups(const std::string &groupPath, std::vector<std::string> &groupNames,
                 std::vector<std::string> &groupPaths);

  bool getSections(const std::string &groupPath, std::vector<std::string> &sections);

  bool getSectionValueNames(const std::string &groupPath, const std::string &sectionName,
                            std::vector<std::string> &names);

 private:
  CConfigGroup *getGroupFromPath(const std::string &groupPath);

  CConfigGroup *getGroup(const std::string &groupName);
};

class CConfigGroup {
  typedef std::list<CConfigGroup *>   CConfigGroupList;
  typedef std::list<CConfigSection *> CConfigSectionList;

 private:
  std::string         path_;
  std::string         name_;
  std::string         hierName_;
  std::string         file_;
  CConfigGroupList    groups_;
  CConfigSectionList  sections_;
  CConfigFile        *config_file_;

 public:
  CConfigGroup(const std::string &path, const std::string &name, const std::string &hierName);
 ~CConfigGroup();

  void save();

  CConfigGroup   *getGroup(const std::string &groupName);
  CConfigSection *getSection(const std::string &sectionName);

  bool getValue(const std::string &sectionName, const std::string &name, std::string &value);
  bool setValue(const std::string &sectionName, const std::string &name, const std::string &value);

  bool isName(const std::string &name) const { return name_ == name; }

  bool getGroups(std::vector<std::string> &groupNames, std::vector<std::string> &groupPaths);

  bool getSections(std::vector<std::string> &sections);

  bool getSectionValueNames(const std::string &sectionName, std::vector<std::string> &names);

 private:
  CConfigGroup(const CConfigGroup &rhs);
  CConfigGroup &operator=(const CConfigGroup &rhs);
};

class CConfigSection {
 private:
  std::string           name_;
  CAutoPtr<CConfigFile> config_file_;

 public:
  CConfigSection(const std::string &name, CConfigFile *config_file);
 ~CConfigSection();

  bool isName(const std::string &name) const { return name_ == name; }

  bool getValue(const std::string &name, std::string &value);
  bool setValue(const std::string &name, const std::string &value);

  bool getValueNames(std::vector<std::string> &names);
};

#endif
