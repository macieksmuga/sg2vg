/*
 * Copyright (C) 2015 by Glenn Hickey (hickey@soe.ucsc.edu)
 *
 * Released under the MIT license, see LICENSE.cactus
 */

#ifndef _JSON2SG_H
#define _JSON2SG_H

#include <vector>
#include <map>
#include <string>
#include <stdexcept>

#include "rapidjson/document.h"

#include "sidegraph.h"

/** put all JSON -> In-memory-sidegraph conversion in one place.  
We are not taking advantage of any schemas or anything, so expected
format is hard coded in each parse function.

Top-level parse functions check for errors and return -1 if any problems are
found.  Lower-level functions will probably assert fail or throw exception.
*/
class JSON2SG
{
public:
   JSON2SG();
   ~JSON2SG();

   /** Parse sequences array.  Note, caller is responsible for freeing seqs.
    * returns number of sequences or -1 if (top level) error */
   int parseSequences(const char* buffer, std::vector<SGSequence*>& outSeqs,
                      std::vector<std::string>& outBases,
                      int& outNextPageToken);

   /** Parse single sequence. */
   SGSequence parseSequence(const rapidjson::Value& val);

   /** Parse references into id->name map */
   int parseReferences(const char* buffer, std::map<int, std::string>& outMap,
                       int& outNextPageToken);

   /** Parse squence bases. 
    * returns number of bases or -1 if error */
   int parseBases(const char* buffer, std::string& outBases);

   /** Parse joins array.  Note, caller is responsible for freeing joins.
    * returns number of joins or -1 if (top-level) error */
   int parseJoins(const char* buffer, std::vector<SGJoin*>& outJoins,
                  int& outNextPageToken);

   /** Parse single join.  Note, caller responsible for freeing join */
   SGJoin* parseJoin(const rapidjson::Value& val);

   /** Prase side */
   SGSide parseSide(const rapidjson::Value& val);

   /** Parse Position */
   SGPosition parsePosition(const rapidjson::Value& val);

   /** Parse out Allele IDs out of alleles array */
   int parseAlleleIDs(const char* buffer, std::vector<int>& outAlleleIDs,
                      int& outNextPageToken);

   /** Parse Allele.
    * returns number of segments in path or -1 if error */
   int parseAllele(const char* buffer, int& outID,
                   std::vector<SGSegment>& outPath,
                   int& outVariantSetID, std::string& outName);

   /** Parse Segment Path */
   int parseAllelePath(const rapidjson::Value& val,
                       std::vector<SGSegment>& outPath);

   /** Parse Segment */
   SGSegment parseSegment(const rapidjson::Value& val);

   /** JSon data always seems to be in strings... centralize code to 
    * read string field into other type with error handling.  */
   template <typename T>
   T extractStringVal(const rapidjson::Value& val, const char* field);

   /** get the nextPageToken.  It's stored as a string.  We convert it to
    * int with following convetion: -1 = NULL, -2 = Error */
   int getNextPageToken(const rapidjson::Value& val);

protected:
   // hmm maybe i shouldnt be a class
};

template <typename T> inline
T JSON2SG::extractStringVal(const rapidjson::Value& val, const char* field)
{
  if (!val.HasMember(field))
  {
    throw std::runtime_error(std::string(
                               "Error parsing JSON field: ") + field);
  }
  const rapidjson::Value& v = val[field];
  if (!v.IsString())
  {
    throw std::runtime_error(std::string(
                               "Error parsing JSON field: ") + field);
  }
  std::stringstream ss;
  ss << v.GetString();
  T ret;
  ss >> ret;
  return ret;
}

#endif
