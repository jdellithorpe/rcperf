/* Copyright (c) 2009-2015 Stanford University
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR(S) DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>

#include <cmath>
#include <iostream>
#include <fstream>

#include "ClusterMetrics.h"
#include "Context.h"
#include "Cycles.h"
#include "Dispatch.h"
#include "ShortMacros.h"
#include "Crc32C.h"
#include "ObjectFinder.h"
#include "OptionParser.h"
#include "RamCloud.h"
#include "Tub.h"
#include "IndexLookup.h"
#include "TableEnumerator.h"
#include "Transaction.h"

using namespace RAMCloud;

class List {
  PUBLIC:
    RamCloud* client;
    uint64_t tableId;
    uint32_t head_segment_size;
    uint32_t key_size;

    List(RamCloud* client, uint64_t tableId, uint32_t head_segment_size) : 
      client(client),
      tableId(tableId),
      head_segment_size(head_segment_size),
      key_size(30) {
     
      char key[key_size];
      memset(key, 0, key_size);
      sprintf(key, "%d", 0); 
      char value[sizeof(uint32_t)];
      memset(value, 0, sizeof(uint32_t));
      client->write(tableId, key, key_size, value, sizeof(uint32_t));
    }

    void append(char* data, uint32_t size) {
      char key[key_size];
      memset(key, 0, key_size);
      sprintf(key, "%d", 0); 
      bool exists;
      Buffer value;
      client->read(tableId, key, key_size, &value, NULL, NULL, &exists);
      if (!exists) {
        printf("ERROR: Head segment does not exist\n");
        return;
      } 

      char* headSeg = (char*)value.getRange(0, value.size());
      uint32_t numTailSegs = *value.getOffset<uint32_t>(0);
      uint32_t newHeadSegSize = value.size() + sizeof(uint32_t) + size;

      if (newHeadSegSize - sizeof(uint32_t) > head_segment_size) {
        // Split!
        // Write new empty head seg with updated metadata.
        char newHeadSeg[sizeof(uint32_t)];
        numTailSegs++;
        memcpy(newHeadSeg, &numTailSegs, sizeof(uint32_t));
        client->write(tableId, key, key_size, newHeadSeg, sizeof(uint32_t));

        // Write new tail segment
        memset(key, 0, key_size);
        sprintf(key, "%d", numTailSegs);

        uint32_t newTailSegSize = newHeadSegSize - sizeof(uint32_t);
        char newTailSeg[newTailSegSize];

        uint32_t offset = 0;
        // Insert the new value at the front
        memcpy(newTailSeg + offset, &size, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(newTailSeg + offset, data, size);
        offset += size;
        // Put the original data at the end
        memcpy(newTailSeg + offset, headSeg + sizeof(uint32_t), value.size() - sizeof(uint32_t));
        client->write(tableId, key, key_size, newTailSeg, newTailSegSize);
      } else {
        // No split
        char newHeadSeg[newHeadSegSize];
        uint32_t offset = 0;
        // Copy the head segment meta data portion
        memcpy(newHeadSeg + offset, headSeg, sizeof(uint32_t));
        // Insert the new value at the front
        offset += sizeof(uint32_t);
        memcpy(newHeadSeg + offset, &size, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(newHeadSeg + offset, data, size);
        offset += size;
        // Put the original data at the end
        memcpy(newHeadSeg + offset, headSeg + sizeof(uint32_t), value.size() - sizeof(uint32_t));
        client->write(tableId, key, key_size, newHeadSeg, newHeadSegSize);
      }
    }

    void check() {
      // Check the list
      char key[key_size];
      memset(key, 0, key_size);
      sprintf(key, "%d", 0); 
      bool exists;
      Buffer value;
      client->read(tableId, key, key_size, &value, NULL, NULL, &exists);
      if (!exists) {
        printf("ERROR: Head segment does not exist\n");
        return;
      } 

      char* headSeg = (char*)value.getRange(0, value.size());
      uint32_t numTailSegs = *value.getOffset<uint32_t>(0);

      printf("List contains %d tail segments\n", numTailSegs);

      printf("Head Segment Contents:\n");
      uint32_t offset = sizeof(uint32_t);
      uint32_t count = 0;
      while(offset < value.size()) {
        uint32_t size = *value.getOffset<uint32_t>(offset);
        offset += sizeof(uint32_t);
        char* element = value.getOffset<char>(offset);
        offset += size;
        count++;
      }

      printf("\tSeg has %d elements\n", count);

      for (int i = numTailSegs; i > 0; i--) {
        memset(key, 0, key_size);
        sprintf(key, "%d", i); 
        bool exists;
        Buffer value;
        client->read(tableId, key, key_size, &value, NULL, NULL, &exists);
        if (!exists) {
          printf("ERROR: Tail segment %d does not exist\n", i);
          return;
        } 

        printf("Tail Segment %d Contents:\n", i);
        uint32_t offset = 0;
        uint32_t count = 0;
        while(offset < value.size()) {
          uint32_t size = *value.getOffset<uint32_t>(offset);
          offset += sizeof(uint32_t);
          char* element = value.getOffset<char>(offset);
          offset += size;
          count++;
        }

        printf("\tSeg has %d elements\n", count);
      }
    }
};

int
main(int argc, char *argv[])
try
{
    int clientIndex;
    int numClients;
    int replicas;
    std::string configFilename;

    // Set line buffering for stdout so that printf's and log messages
    // interleave properly.
    setvbuf(stdout, NULL, _IOLBF, 1024);

    // need external context to set log levels with OptionParser
    Context context(false);

    OptionsDescription clientOptions("listperf");
    clientOptions.add_options()

        // These first two options are currently ignored. They're here so that
        // this script can be run with cluster.py.
        ("clientIndex",
         ProgramOptions::value<int>(&clientIndex)->
            default_value(0),
         "Index of this client (first client is 0; currently ignored)")
        ("numClients",
         ProgramOptions::value<int>(&numClients)->
            default_value(1),
         "Total number of clients running (currently ignored)")

        ("replicas",
         ProgramOptions::value<int>(&replicas),
         "Number of replicas configured for given RAMCloud cluster.")
        ("config",
         ProgramOptions::value<std::string>(&configFilename),
         "Configuration file for experiment specification.");
    
    OptionParser optionParser(clientOptions, argc, argv);
    context.transportManager->setSessionTimeout(
            optionParser.options.getSessionTimeout());

    LOG(NOTICE, "Connecting to %s",
        optionParser.options.getCoordinatorLocator().c_str());

    string locator = optionParser.options.getExternalStorageLocator();
    if (locator.size() == 0) {
        locator = optionParser.options.getCoordinatorLocator();
    }

    RamCloud client(&optionParser.options);

    // Default values for experiment parameters
    uint32_t element_range_start = 30;
    uint32_t element_range_end = 30;
    uint32_t element_points = 1;
    std::string element_points_mode = "linear";
    uint32_t head_segment_range_start = 100;
    uint32_t head_segment_range_end = 100;
    uint32_t head_segment_points = 1;
    std::string head_segment_points_mode = "linear";
    uint32_t samples_per_point = 1000;

    std::ifstream cfgFile(configFilename);
    std::string line;
    std::string op;
    while (true) {
      if (cfgFile.eof()) {
        printf("End of experiments\n");
        return 0;
      }

      bool foundCfg = false;
      while (std::getline(cfgFile, line)) {
        if (line[0] == '[') {
          op = line.substr(1, line.find_last_of(']') - 1);
          foundCfg = true;
        } else if (line[0] == '#') {
          // skip
          continue;
        } else if (line.size() == 0) {
          if (foundCfg == false)
            continue;
          else
            break;
        } else {
          std::string var_name = line.substr(0, line.find_first_of(' '));
          
          if (var_name.compare("element_range_start") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            element_range_start = var_int_value;
          } else if (var_name.compare("element_range_end") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            element_range_end = var_int_value;
          } else if (var_name.compare("element_points") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            element_points = var_int_value;
          } else if (var_name.compare("element_points_mode") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            element_points_mode = var_value;
          } else if (var_name.compare("head_segment_range_start") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            head_segment_range_start = var_int_value;
          } else if (var_name.compare("head_segment_range_end") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            head_segment_range_end = var_int_value;
          } else if (var_name.compare("head_segment_points") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            head_segment_points = var_int_value;
          } else if (var_name.compare("head_segment_points_mode") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            head_segment_points_mode = var_value;
          } else if (var_name.compare("samples_per_point") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            samples_per_point = var_int_value;
          } else {
            printf("ERROR: Unknown parameter: %s\n", var_name.c_str());
            return 1;
          }
        }
      }

      if (foundCfg == false) {
        printf("End of experiments\n");
        return 0;
      }

      std::vector<uint32_t> element_sizes; 
      std::vector<uint32_t> head_segment_sizes; 

      if (element_points > 1) {
        if (element_points_mode.compare("linear") == 0) {
          uint32_t step_size = 
            (element_range_end - element_range_start) / (element_points - 1);

          for (int i = element_range_start; i <= element_range_end; i += step_size) 
            element_sizes.push_back(i);
        } else if (element_points_mode.compare("geometric") == 0) {
          double c = pow(10, log10((double)element_range_end/(double)element_range_start) / (double)(element_points - 1));
          for (int i = element_range_start; i <= element_range_end; i *= c)
            element_sizes.push_back(i);
        } else {
          printf("ERROR: Unknown points mode: %s\n", element_points_mode.c_str());
          return 1;
        }
      } else {
        element_sizes.push_back(element_range_start);
      }

      if (head_segment_points > 1) {
        if (head_segment_points_mode.compare("linear") == 0) {
          uint32_t step_size = 
            (head_segment_range_end - head_segment_range_start) / (head_segment_points - 1);

          for (int i = head_segment_range_start; i <= head_segment_range_end; i += step_size) 
            head_segment_sizes.push_back(i);
        } else if (head_segment_points_mode.compare("geometric") == 0) {
          double c = pow(10, log10((double)head_segment_range_end/(double)head_segment_range_start) / (double)(head_segment_points - 1));
          for (int i = head_segment_range_start; i <= head_segment_range_end; i *= c)
            head_segment_sizes.push_back(i);
        } else {
          printf("ERROR: Unknown points mode: %s\n", head_segment_points_mode.c_str());
          return 1;
        }
      } else {
        head_segment_sizes.push_back(head_segment_range_start);
      }

      if (op.compare("append") == 0) {
        uint64_t tableId = client.createTable("test");

        for (int es_idx = 0; es_idx < element_sizes.size(); es_idx++) {
          uint32_t element_size = element_sizes[es_idx];

          // Open data file for writing.
          FILE * datFile;
          char filename[128];
          sprintf(filename, "append.spp_%d.es_%d.csv", samples_per_point, element_size);
          datFile = fopen(filename, "w");
          fprintf(datFile, "%12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s\n", 
              "SegSize",
              "Avg",
              "1th",
              "2th",
              "5th",
              "10th",
              "25th",
              "50th",
              "75th",
              "90th",
              "95th",
              "98th",
              "99th");

          for (int hs_idx = 0; hs_idx < head_segment_sizes.size(); hs_idx++) {
            uint32_t head_segment_size = head_segment_sizes[hs_idx];
            printf("Append Test: element_size: %dB, head_segment_size: %dB\n", element_size, head_segment_size);
            
            List list(&client, tableId, head_segment_size);

            char element[element_size];

            uint64_t latency[samples_per_point];
            for (int i = 0; i < samples_per_point; i++) {
              uint64_t start = Cycles::rdtsc();
              list.append(element, element_size);
              uint64_t end = Cycles::rdtsc();
              latency[i] = Cycles::toNanoseconds(end-start);
            }

            std::vector<uint64_t> latencyVec(latency, latency+samples_per_point);

            std::sort(latencyVec.begin(), latencyVec.end());

            uint64_t sum = 0;
            for (int i = 0; i < samples_per_point; i++) {
              sum += latencyVec[i];
            }

            fprintf(datFile, "%12d %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f\n", 
                head_segment_size,
                (double)sum / (double)samples_per_point,
                latencyVec[samples_per_point*1/100]/1000.0,
                latencyVec[samples_per_point*2/100]/1000.0,
                latencyVec[samples_per_point*5/100]/1000.0,
                latencyVec[samples_per_point*10/100]/1000.0,
                latencyVec[samples_per_point*25/100]/1000.0,
                latencyVec[samples_per_point*50/100]/1000.0,
                latencyVec[samples_per_point*75/100]/1000.0,
                latencyVec[samples_per_point*90/100]/1000.0,
                latencyVec[samples_per_point*95/100]/1000.0,
                latencyVec[samples_per_point*98/100]/1000.0,
                latencyVec[samples_per_point*99/100]/1000.0);
            fflush(datFile);
          }

          fclose(datFile);
        }

        client.dropTable("test");
      } // op == "append"
    } // while (true) // cfg file reading
  
    return 0;
} catch (RAMCloud::ClientException& e) {
    fprintf(stderr, "RAMCloud exception: %s\n", e.str().c_str());
    return 1;
} catch (RAMCloud::Exception& e) {
    fprintf(stderr, "RAMCloud exception: %s\n", e.str().c_str());
    return 1;
}
