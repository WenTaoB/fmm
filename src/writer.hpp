/**
 * Content
 * Definition of ResultWriter Class, which contains functions for
 * writing the results.
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_WRITER_HPP /* Currently not used */
#define MM_WRITER_HPP
#include <iostream>
#include <fstream>
#include "types.hpp"
#include "multilevel_debug.h"
#include "config.hpp"
#include "network.hpp"

namespace MM
{
namespace IO {

/**
 *  The result writer is constructed with an output file name
 *  and a network pointer.
 *
 *  The network pointer is required because the complete path is actually
 *  a vector of indexes of the edges in the SHP file, which needs to be
 *  converted into a vector of IDs.
 */
class ResultWriter
{
public:
    /**
     * Constructor
     * @param result_file, the path to an output file
     * @param network_ptr, a pointer to the network
     */
    ResultWriter(const std::string &result_file, Network *network_ptr, ResultConfig &config_arg):
        m_fstream_ptr(new std::ofstream(result_file)),
        m_fstream(*m_fstream_ptr), m_network_ptr(network_ptr),config(config_arg)
    {
        std::cout << "Write result to file: " << result_file << '\n';
        write_header();
    };
    // Destructor
    ~ResultWriter() {
        delete m_fstream_ptr;
    };

    /**
     *  Write map matching result for a trajectory
     * @param tr_id: id of trajectory
     * @param ogeom: original geometry of the trajectory OGRLineString*
     * @param o_path_ptr: pointer to the optimal path (sequence of candidates)
     * @param c_path_ptr: pointer to the complete path (sequence of edge ids)
     * @param mgeom: the geometry of the matched path (untraversed path removed in complete path)
     */
    void write_result(int tr_id, OGRLineString *ogeom, O_Path *o_path_ptr, T_Path *t_path_ptr, OGRLineString *mgeom){
        DEBUG(2) std::cout << __FILE__ << "    Line" << __LINE__ << ":    " << __FUNCTION__ << '\n';
        std::stringstream buf;
        buf << tr_id;
        if (config.write_ogeom){
            buf << ";";
            if (ogeom != nullptr) {
                char *wkt;
                ogeom->exportToWkt(&wkt);
                buf << wkt;
                CPLFree(wkt);
            }
        }
        if (config.write_opath) {
            buf << ";";
            write_o_path(buf,o_path_ptr);
        }
        if (config.write_error) {
            buf << ";";
            write_gps_error(buf,o_path_ptr);
        }
        if (config.write_offset) {
            buf << ";";
            write_offset(buf,o_path_ptr);
        }
        // Distance traversed which could be more complicated.
        if (config.write_spdist) {
            buf << ";";
            write_spdist(buf,o_path_ptr);
        }
        if (config.write_pgeom){
            buf << ";";
            write_pgeom(buf,o_path_ptr);
        }
        // Write fields related with cpath
        if (config.write_cpath) {
            buf << ";";
            write_cpath(buf,t_path_ptr);
        }
        if (config.write_tpath) {
            buf << ";";
            write_tpath(buf,t_path_ptr);
        }
        if (config.write_mgeom) {
            buf << ";";
            if (mgeom != nullptr) {
                char *wkt;
                mgeom->exportToWkt(&wkt);
                buf << wkt;
                CPLFree(wkt);
            } 
        }
        buf << '\n';
        // It seems that the one below is important to ensure the buffer control flow works
        // as expected. 
        #pragma omp critical
        m_fstream << buf.rdbuf();
    };

private:

    void write_header() {
        std::string header = "id";
        if (config.write_ogeom) header+=";ogeom";
        if (config.write_opath) header+=";opath";
        if (config.write_error) header+=";error";
        if (config.write_offset) header+=";offset";
        if (config.write_spdist) header+=";spdist";
        if (config.write_pgeom) header+=";pgeom";
        if (config.write_cpath) header+=";cpath";
        if (config.write_tpath) header+=";tpath";
        if (config.write_mgeom) header+=";mgeom";
        m_fstream << header << '\n';
    };    
    
    // Write the optimal path
    void write_o_path(std::stringstream &buf,O_Path *o_path_ptr)
    {
        DEBUG(2) std::cout << __FILE__ << "    Line" << __LINE__ << ":    " << __FUNCTION__ << '\n';
        if (o_path_ptr == nullptr) {
            DEBUG(2) std::cout << "Matched path NULL" << '\n';
            return;
        };
        int N = o_path_ptr->size();
        for (int i = 0; i < N - 1; ++i)
        {
            buf << (*o_path_ptr)[i]->edge->id_attr << ",";
        }
        buf << (*o_path_ptr)[N - 1]->edge->id_attr;
        DEBUG(3) std::cout << "Finish writing Optimal path" << '\n';
    };
    
    void write_offset(std::stringstream &buf,O_Path *o_path_ptr)
    {
        DEBUG(2) std::cout << __FILE__ << "    Line" << __LINE__ << ":    " << __FUNCTION__ << '\n';
        if (o_path_ptr == nullptr) {
            DEBUG(2) std::cout << "Matched path NULL" << '\n';
            return;
        };
        int N = o_path_ptr->size();
        for (int i = 0; i < N - 1; ++i)
        {
            buf << (*o_path_ptr)[i]->offset<< ",";
        }
        buf << (*o_path_ptr)[N - 1]->offset;
        DEBUG(3) std::cout << "Finish writing Optimal path" << '\n';
    };
    
    void write_spdist(std::stringstream &buf,O_Path *o_path_ptr)
    {
        DEBUG(2) std::cout << __FILE__ << "    Line" << __LINE__ << ":    " << __FUNCTION__ << '\n';
        if (o_path_ptr == nullptr) {
            DEBUG(2) std::cout << "Matched path NULL" << '\n';
            return;
        };
        int N = o_path_ptr->size();
        for (int i = 0; i < N - 1; ++i)
        {
            buf << (*o_path_ptr)[i]->sp_dist<< ",";
        }
        buf << (*o_path_ptr)[N - 1]->sp_dist;
        DEBUG(3) std::cout << "Finish writing Optimal path" << '\n';
    };
    
    void write_gps_error(std::stringstream &buf,O_Path *o_path_ptr)
    {
        DEBUG(2) std::cout << __FILE__ << "    Line" << __LINE__ << ":    " << __FUNCTION__ << '\n';
        if (o_path_ptr == nullptr) {
            DEBUG(2) std::cout << "Matched path NULL" << '\n';
            return;
        };
        int N = o_path_ptr->size();
        for (int i = 0; i < N - 1; ++i)
        {
            buf << (*o_path_ptr)[i]->dist<< ",";
        }
        buf << (*o_path_ptr)[N - 1]->dist;
        DEBUG(3) std::cout << "Finish writing Optimal path" << '\n';
    };
    
    // Write the optimal path
    void write_pgeom(std::stringstream &buf,O_Path *o_path_ptr)
    {
        DEBUG(2) std::cout << __FILE__ << "    Line" << __LINE__ << ":    " << __FUNCTION__ << '\n';
        if (o_path_ptr == nullptr) {
            DEBUG(2) std::cout << "Matched path NULL" << '\n';
            return;
        };
        int N = o_path_ptr->size();
        OGRLineString pline;
        // Create a linestring from matched point
        for (int i = 0; i < N; ++i)
        {
            OGRLineString *edge_geom = (*o_path_ptr)[i]->edge->geom;
            double px = 0;
            double py = 0; 
            ALGORITHM::locate_point_by_offset(edge_geom,(*o_path_ptr)[i]->offset,&px,&py);
            pline.addPoint(px,py);
        }
        if (!pline.IsEmpty()) {
            char *wkt;
            pline.exportToWkt(&wkt);
            buf << wkt;
            CPLFree(wkt);
        } 
        DEBUG(3) std::cout << "Finish writing Optimal path" << '\n';
    };
    
    // Write the complete path
    void write_cpath(std::stringstream &buf,T_Path *t_path_ptr) {
        if (t_path_ptr == nullptr) return;
        C_Path *c_path_ptr = &(t_path_ptr->cpath);
        int N = c_path_ptr->size();
        for (int i = 0; i < N - 1; ++i)
        {
            buf << m_network_ptr->get_edge_id_attr((*c_path_ptr)[i]) << ",";
        }
        buf << m_network_ptr->get_edge_id_attr((*c_path_ptr)[N - 1]);
    };
        
    // Write the traversed path separated by vertical bar
    void write_tpath(std::stringstream &buf,T_Path *t_path_ptr) {
        if (t_path_ptr == nullptr) return;
        // Iterate through consecutive indexes and write the traversed path
        int J = t_path_ptr->indices.size(); 
        for (int j=0;j<J-1;++j){
            int a = t_path_ptr->indices[j];
            int b = t_path_ptr->indices[j+1];
            for (int i=a;i<b;++i){
                buf << m_network_ptr->get_edge_id_attr(t_path_ptr->cpath[i]);
                buf << ",";
            }
            buf << m_network_ptr->get_edge_id_attr(t_path_ptr->cpath[b]);
            if (j<J-2){
                // Last element should not have a bar
                buf << "|";
            }
        }
    };
    
    std::ostream *m_fstream_ptr;
    std::ostream &m_fstream;
    Network *m_network_ptr;
    ResultConfig config;
}; // ResultWriter
}; //IO
} //MM
#endif // MM_WRITER_HPP
