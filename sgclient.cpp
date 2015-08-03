/*
 * Copyright (C) 2015 by Glenn Hickey (hickey@soe.ucsc.edu)
 *
 * Released under the MIT license, see LICENSE.cactus
 */

#include <iostream>
#include <sstream>

#include "rapidjson/document.h"    
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "sgclient.h"
#include "json2sg.h"


using namespace std;
using namespace rapidjson;

const string SGClient::CTHeader = "Content-Type: application/json";

SGClient::SGClient() : _sg(0)
{

}

SGClient::~SGClient()
{
  erase();
}

void SGClient::erase()
{
  delete _sg;
  _url = "";
  _sg = 0;
}

void SGClient::setURL(const string& baseURL)
{
  erase();
  
  _url = baseURL;
  assert(_url.length() > 1);
  if (_url.back() == '/')
  {
    _url.pop_back();
  }

  size_t lastSlash = _url.find_last_of("/");
  size_t lastV = _url.find_last_of("vV");

  if (lastV == string::npos ||
      (lastSlash != string::npos && lastV <= lastSlash) ||
      lastV + 6 < _url.length())
  {
    throw runtime_error("Version not detected at end of URL");
  }

  _sg = new SideGraph();
}

int SGClient::downloadSequences(vector<const SGSequence*>& outSequences,
                                int idx, int numSequences,
                                int referenceSetID, int variantSetID)
{
  outSequences.clear();
    
  string postOptions = getPostOptions(idx, numSequences, referenceSetID,
                                      variantSetID);

  string path = "/sequences/search";

  // Send the Request
  const char* result = _download.postRequest(_url + path,
                                             vector<string>(1, CTHeader),
                                             postOptions);

  // Parse the JSON output into a Sequences array and add it to the side graph
  JSON2SG parser;
  vector<SGSequence*> sequences;
  parser.parseSequences(result, sequences);
  for (int i = 0; i < sequences.size(); ++i)
  {
    sg_int_t originalID = sequences[i]->getID();

    // no name in the json.  so we give it a name based on original id
    stringstream ss;
    ss << "Seq" << originalID;
    sequences[i]->setName(ss.str());
    
    // store map to original id as Side Graph interface requires
    // ids be [0,n) which may be unessarily strict.  Too lazy right
    // now to track down SGExport code that depends on this..
    const SGSequence* addedSeq = _sg->addSequence(sequences[i]);
    addSeqIDMapping(originalID, addedSeq->getID());

    outSequences.push_back(addedSeq);
  }

  return outSequences.size();
}

int SGClient::downloadBases(sg_int_t sgSeqID, string& outBases, int start,
                            int end)
{
  outBases.clear();

  const SGSequence* seq = _sg->getSequence(sgSeqID);
  if (seq == NULL)
  {
    stringstream ss;
    ss << "Unable to downloadBases for sgSeqID " << sgSeqID << " because"
       << " sequence was never downloaded using downloadSequecnes";
    throw runtime_error(ss.str());
  }
  int origID = getOriginalSeqID(sgSeqID);

  int queryLen = seq->getLength();
  stringstream opts;
  opts << "/sequences/" << origID << "/bases";
  if (start != 0 && end != -1)
  {
    if (start < 0 || end < 0 || end <= start || end > seq->getLength())
    {
      stringstream ss;
      ss << "start=" << start << ", end=" << end << " invalid for sequence"
         << " id=" << origID << " with length " << seq->getLength();
      throw runtime_error(ss.str());
    }
    opts << "?start=" << start << "\\&end=" << end;
    queryLen = end - start;
  }

  string path = opts.str();
  
  // Send the Request
  const char* result = _download.getRequest(_url + path,
                                            vector<string>());

  // Parse the JSON output into a string
  JSON2SG parser;
  parser.parseBases(result, outBases);

  if (outBases.length() != queryLen)
  {
    stringstream ss;
    ss << "Tried to download " << queryLen << " bases for sequence "
       << origID << " but got " << outBases.length() << " bases.";
    throw runtime_error(ss.str());
  }

  return outBases.length();
}

int SGClient::downloadJoins(vector<const SGJoin*>& outJoins,
                            int idx, int numJoins,
                            int referenceSetID, int variantSetID)
{
  outJoins.clear();
  
  string postOptions = getPostOptions(idx, numJoins, referenceSetID,
                                      variantSetID);
  string path = "/joins/search";

  // Send the Request
  const char* result = _download.postRequest(_url + path,
                                             vector<string>(1, CTHeader),
                                             postOptions);

  // Parse the JSON output into a Joins array and add it to the side graph
  JSON2SG parser;
  vector<SGJoin*> joins;
  parser.parseJoins(result, joins);
  for (int i = 0; i < joins.size(); ++i)
  {
    mapSeqIDsInJoin(*joins[i]);
    outJoins.push_back(_sg->addJoin(joins[i]));
  }

  return outJoins.size();
}

int SGClient::downloadAllele(int alleleID, std::vector<SGSegment>& outPath,
                             int& outVariantSetID, std::string& outName)
{
  outPath.clear();

  stringstream opts;
  opts << "/alleles/" << alleleID;
  string path = opts.str();

  const char* result = _download.getRequest(_url + path,
                                            vector<string>());

  int outID;
  JSON2SG parser;
  int ret = parser.parseAllele(result, outID, outPath, outVariantSetID,
                               outName);

  if (outID != alleleID)
  {
    throw runtime_error("AlleleID mismatch");
  }

  return ret;
}

string SGClient::getPostOptions(int pageToken,
                              int pageSize,
                              int referenceSetID,
                              int variantSetID) const
{
   // Build JSON POST Options
  Document doc;
  doc.Parse("{}");
  Value nv;
  assert(nv.IsNull());
  doc.AddMember("pageSize", nv, doc.GetAllocator());
  doc["pageSize"].SetInt64(pageSize);
  doc.AddMember("pageToken", nv, doc.GetAllocator());
  if (pageToken > 0)
  {
    doc["pageToken"].SetInt64(pageToken);
  }
  doc.AddMember("referenceSetId", nv, doc.GetAllocator());
  if (referenceSetID >= 0)
  {
    doc["referenceSetId"].SetInt64(referenceSetID);
  }
  doc.AddMember("variantSetId", nv, doc.GetAllocator());
  if (variantSetID >= 0)
  {
    doc["variantSetId"].SetInt64(variantSetID);
  }
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);
  doc.Accept(writer);
  return buffer.GetString();
}
