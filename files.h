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



/** \brief Write a SpotDiagram object to a file in binary format
 *
 * \param file  output binary file
 * \param spotDiag spot diagram data
 * \return the file reference
 * \relates SpotDiagram
 */
template<int Vsize>
inline fstream& operator<<(fstream& file, DiagramType<Vsize>& diagram )
{
    streamsize bytes=Vsize*sizeof(double)*diagram.m_count;
//    int vecSize=Vsize;
//    file.write((char*)&vecSize, sizeof(int));
    file.write((char*)&diagram, 2*sizeof(int)+ 4*Vsize*sizeof(double));
    file.write((char*)diagram.m_spots, bytes);
    return file;
}

inline fstream& operator<<(fstream& file, WavefrontData& wfData )
{
    int N[2]={(int)wfData.m_WFdata.rows(), (int)wfData.m_WFdata.cols()};
    file.write((char*)N, 2*sizeof(int));
    file.write((char*)wfData.m_bounds.data(), 4*sizeof(double )) ;
    file.write ((char*)wfData.m_WFdata.data(), N[0]*N[1]*sizeof(double));

    return file;
}


/** \brief write a Parameter to a readable output file
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
    if(file.fail()) throw TextFileException("Error while reading Parameter from File", __FILE__, __func__, __LINE__);

    return file;
}

/** \brief write a Parameter to a readable output file
 *
 * \param file  output text file
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

class SolemioFile:public fstream
{
public:
  //  SolemioFile(){}
    ~SolemioFile(){fstream::close();}
    SolemioFile(string filename);        //  :fstream(filename,  ios::in ){} // ouverture en lecture seule
    void skipline(int n=1);
    void getPrefixedString(string& str);
    void getScript(string& str);
    bool check_comment(const string comment);
    bool get_element();
    inline SolemioFile& operator>>(int& i) {*((fstream*)this)>>i; return *this;}
    inline SolemioFile& operator>>(uint32_t& i) {*((fstream*)this)>>i; return *this;}
    inline SolemioFile& operator>>(double& i) {*((fstream*)this)>>i; return *this;}
    SolemioFile& operator>>(ArrayXd&  darray);
    int version;
};



void ReadSolemioFile(string filename);


#endif // FILES_H_INCLUDED
