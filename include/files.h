#ifndef FILES_H_INCLUDED
#define FILES_H_INCLUDED

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           files.h
*
*      \brief         Text and binary files types for saving data
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-11-10  Creation
*      \date         Last update
*

*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <limits>
#include <iomanip>
#include "types.h"


using namespace std;

/** \brief stream file with operators for storing OptiX elements  in a human readable way
 */
class TextFile:protected fstream
{
public:
    TextFile(){}
    ~TextFile(){fstream::close();}
    TextFile(string filename, ios_base::openmode mode= ios::in | ios::out ){open(filename,mode);}
    inline  void open(string filename, ios_base::openmode mode= ios::in | ios::out )
    {
        m_mode=mode & ~ios::binary; // force binary flag out
        fstream::open(filename, m_mode);
       // *this << dec << scientific;
       ios::fmtflags ff;
        ff = flags();
        ff &= ~ios::basefield; //& ~ios::floatfield;   // unset basefield and floatfield bits
        ff |= ios::dec ;//| ios::showpoint; //| ios::scientific;

        flags(ff);
        precision(12);
    }
    inline TextFile& flush(){fstream::flush(); return *this;}
    inline void close(){fstream::close();}
    inline bool fail(){return fstream::fail();}
    inline bool eof(){return fstream::eof();}
    inline bool is_open(){return fstream::is_open();}
    inline TextFile& seekg( pos_type pos ){fstream::seekg(pos); return *this;}


    /** \brief Insert this char in the stream <b> without NULL separator </b>
     * \return a reference to the stream */
    inline TextFile& operator<<(char num ){
       fstream* base=static_cast<fstream*>(this);
       *base<< num;
       return *this;
    }

    /** \brief insert the operand followed by a NULL separator to the stream (this is the default behaviour of operator<<() but for type char)
     * \return a reference to the stream */
    inline TextFile& operator<<(double num ){
       fstream* base=static_cast<fstream*>(this);
       *base<< num << '\0';
       return *this;
    }
    /** \brief \sa operator<<(double) */
    inline TextFile& operator<<(short num ){
       fstream* base=static_cast<fstream*>(this);
       *base<< num << '\0';
       return *this;
    }

    inline TextFile& operator<<(unsigned short num ){
       fstream* base=static_cast<fstream*>(this);
       *base<< num << '\0';
       return *this;
    }

    inline TextFile& operator<<(int num ){
       fstream* base=static_cast<fstream*>(this);
       *base<< num << '\0';
       return *this;
    }

    inline TextFile& operator<<(unsigned int num ){
       fstream* base=static_cast<fstream*>(this);
       *base<< num << '\0';
       return *this;
    }

    inline TextFile& operator<<(unsigned long long  num ){
       fstream* base=static_cast<fstream*>(this);
       *base<< num << '\0';
       return *this;
    }

    inline TextFile& operator<<(char* num ){
       fstream* base=static_cast<fstream*>(this);
       *base<< num << '\0';
       return *this;
    }

    inline TextFile& operator<<(string str ){
       fstream* base=static_cast<fstream*>(this);
       *base <<str << '\0';
       return *this;
    }

    inline TextFile& operator>>(string& str)
    {
        fstream::get(m_rdBuf, sizeof(m_rdBuf),'\0');
        if(fstream::fail()) fstream::clear(fstream::rdstate() &ios::badbit & ios::eofbit);
        str=string(m_rdBuf);
        fstream::ignore(1);
        return * this;
    }

    inline TextFile& operator>>(double& num)
    {
        fstream::get(m_rdBuf, sizeof(m_rdBuf),'\0');
        if(fstream::fail()) fstream::clear(fstream::rdstate() &ios::badbit & ios::eofbit);
        num=atof(m_rdBuf);
        fstream::ignore(1);
        return * this;
    }

    inline TextFile& operator>>(int& num)
    {
        fstream::get(m_rdBuf, sizeof(m_rdBuf),'\0');
        if(fstream::fail()) fstream::clear(fstream::rdstate() &ios::badbit & ios::eofbit);
        num=atoi(m_rdBuf);
        fstream::ignore(1);
        return * this;
    }

    inline TextFile& operator>>(unsigned int& num)
    {
        fstream::get(m_rdBuf, sizeof(m_rdBuf),'\0');
        if(fstream::fail()) fstream::clear(fstream::rdstate() &ios::badbit & ios::eofbit);
        num=atoi(m_rdBuf);
        fstream::ignore(1);
        return * this;
    }


    inline TextFile& operator>>(long& num)
    {
        fstream::get(m_rdBuf, sizeof(m_rdBuf),'\0');
        if(fstream::fail()) fstream::clear(fstream::rdstate() &ios::badbit & ios::eofbit);
        num=atoll(m_rdBuf);
        fstream::ignore(1);
        return * this;
    }

    inline TextFile& operator>>(unsigned long& num)
    {
        fstream::get(m_rdBuf, sizeof(m_rdBuf),'\0');
        if(fstream::fail()) fstream::clear(fstream::rdstate() &ios::badbit & ios::eofbit);
        num=atoll(m_rdBuf);
        fstream::ignore(1);
        return * this;
    }


    inline TextFile& operator>>(long long& num)
    {
        fstream::get(m_rdBuf, sizeof(m_rdBuf),'\0');
        if(fstream::fail()) fstream::clear(fstream::rdstate() &ios::badbit & ios::eofbit);
        num=atoll(m_rdBuf);
        fstream::ignore(1);
        return * this;
    }

    inline TextFile& operator>>(unsigned long long& num)
    {
        fstream::get(m_rdBuf, sizeof(m_rdBuf),'\0');
        if(fstream::fail()) fstream::clear(fstream::rdstate() &ios::badbit & ios::eofbit);
        num=atoll(m_rdBuf);
        fstream::ignore(1);
        return * this;
    }

    inline TextFile& ignore(int_type delim = '\n' )
    {
        fstream::ignore(numeric_limits<streamsize>::max(), delim);
        return *this;
    }


    ios_base::openmode getMode(){return m_mode;}
    inline bool canRead(){return m_mode & ios::out;}
    inline bool canWrite(){return m_mode &ios::in;}
protected:
    char m_rdBuf[128];
    ios_base::openmode m_mode;
};



// /** \brief Write a SpotDiagram object to a file in binary format
// * \ingroup GlobalCpp
// * \param file  output binary file
// * \param spotDiag spot diagram data
// * \return the file reference
// * \relates SpotDiagram
// */
//template<int Vsize>
//inline fstream& operator<<(fstream& file, DiagramType<Vsize>& diagram )
//{
//    streamsize bytes=Vsize*sizeof(double)*diagram.m_count;
//
//    file.write((char*)&diagram, 4*sizeof(int));
//    file.write((char*) diagram.m_min, Vsize*sizeof(double));
//    file.write((char*) diagram.m_max, Vsize*sizeof(double));
//    file.write((char*) diagram.m_mean, Vsize*sizeof(double));
//    file.write((char*) diagram.m_sigma, Vsize*sizeof(double));
//
//    file.write((char*)diagram.m_spots, bytes);
//    return file;
//}

inline fstream& operator<<(fstream& file, Diagram& diagram )
{
    streamsize bytes=diagram.m_dim*sizeof(double)*diagram.m_count;

    file.write((char*)&diagram, 4*sizeof(int));
    file.write((char*) diagram.m_min, diagram.m_dim*sizeof(double));
    file.write((char*) diagram.m_max, diagram.m_dim*sizeof(double));
    file.write((char*) diagram.m_mean, diagram.m_dim*sizeof(double));
    file.write((char*) diagram.m_sigma, diagram.m_dim*sizeof(double));

    file.write((char*)diagram.m_spots, bytes);
    return file;
}

/** \brief Write a Spot-diagram contained in a C_DiagramStructto a file in binary format
 * \ingroup GlobalCpp
 * \param file  output binary file
 * \param cdiagram spot diagram data
 * \return the file reference
 * \relates SpotDiagram
 */
inline fstream& operator<<(fstream& file, C_DiagramStruct& cdiagram )
{
    streamsize bytes=cdiagram.m_dim*sizeof(double)*cdiagram.m_count;

    file.write((char*)&cdiagram, 4*sizeof(int));
    file.write((char*) cdiagram.m_min, cdiagram.m_dim*sizeof(double));
    file.write((char*) cdiagram.m_max, cdiagram.m_dim*sizeof(double));
    file.write((char*) cdiagram.m_mean, cdiagram.m_dim*sizeof(double));
    file.write((char*) cdiagram.m_sigma, cdiagram.m_dim*sizeof(double));

    file.write((char*)cdiagram.m_spots, bytes);
    return file;
}
/** \brief Write a WavefrontData  object to a file in binary format
 *
 * \param file  output binary file
 * \param wfData Wavefront data
 * \return the file reference
 * \relates WavefrontData and C_WFtype
 */
inline fstream& operator<<(fstream& file, WavefrontData& wfData )
{
    int N[2]={(int)wfData.m_WFdata.rows(), (int)wfData.m_WFdata.cols()};
    file.write((char*)N, 2*sizeof(int));
    file.write((char*)wfData.m_bounds.data(), 4*sizeof(double )) ;
    file.write ((char*)wfData.m_WFdata.data(), N[0]*N[1]*sizeof(double));

    return file;
}


/** \brief write a Parameter to a human-readable output file
 *
 * \param file  output text file
 * \param param reference to the Parameter object
 * \return a reference to the file
 * \relates Parameter
 */
inline TextFile& operator<<(TextFile& file, const Parameter& param)
{
    file << param.value << param.bounds[0] << param.bounds[1] << param.multiplier << (uint32_t)param.type <<
                        (uint32_t)param.group << (uint32_t)param.flags;
    if(file.fail()) throw TextFileException("Error while writing Parameter from File", __FILE__, __func__, __LINE__);

    return file;
}

/** \brief Reads a Parameter from a human-readable input file
 *
 *  All the  parameters fields must be present in the file
 * \param file  input text file
 * \param param reference to the Parameter object
 * \return a reference to the file
 * \relates Parameter
 */
inline TextFile& operator>>( TextFile& file,  Parameter& param)
{   uint32_t t1,t2;
    file >> param.value  >> param.bounds[0] >> param.bounds[1] >> param.multiplier  >> t1 >> t2 >> param.flags;
    param.type =UnitType(t1);
    param.group =ParameterGroup(t2);

    return file;
}

/** \brief Text stream file with appropriate operators for reading elements sored in Solemio data file
 */
class SolemioFile:public fstream
{
public:

    typedef map<uint32_t, SolemioLinkType> LinkMap;
  //  SolemioFile(){}
    ~SolemioFile(){fstream::close();}/**< \brief close the file and delete the file object */
    SolemioFile(string filename);   /**< \brief open the file in read-only mode */     //  :fstream(filename,  ios::in ){} // ouverture en lecture seule
    void skipline(int n=1);/**< \brief skips a number of end-of-line marks */
    void getPrefixedString(string& str);/**< \brief gets a length prefixed string */
    void getScript(string& str);/**< \brief gets a length and 's' prefixed string */
    bool check_comment(const string comment);/**< \brief assert that thee next item in the stream is a comment of given content*/

    /** \brief reads the next solemio surface element and dump its content to  cout. Not all elemnts types are implemented yet
     *
     * \param[in,out] pelemID if not NULL, an equivalent OptiX element will be created. The Handle  the created element or NULL if none was created, will be written at address pointed by pelemID
     * \return false in case of a reading error; true otherwise
     */
    bool get_element(size_t* pelemID=NULL);
    inline SolemioFile& operator>>(int& i) {*((fstream*)this)>>i; return *this;}/**< \brief read an integer value */
    inline SolemioFile& operator>>(uint32_t& i) {*((fstream*)this)>>i; return *this;}/**< \brief read an unsigned integer value */
    inline SolemioFile& operator>>(double& i) {*((fstream*)this)>>i; return *this;}/**< \brief reads a double floating point value */
    SolemioFile& operator>>(ArrayXd&  darray);/**< \brief reads an array of double of known size */
    int version;
    LinkMap iconTable, elemTable;
};



void ReadSolemioFile(string filename);/**< dumps the content of a Solemio file to cout */


/** \brief import the elements of the Solemio file in the current system
 * \ingroup GlobalCpp
 * \param filename full path of the Solemio file
 * \return false if an error occurred and set the OptixLastError, return true otherwise
 *
 */
bool SolemioImport(string filename);


bool LoadConfiguration(string filename);

#endif // FILES_H_INCLUDED
