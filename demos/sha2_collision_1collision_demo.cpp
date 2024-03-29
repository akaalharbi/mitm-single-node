#include "../mitm.hpp"
// #include <algorithm>
#include <random>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iomanip>
// #include <array>
#include <vector>
// #include<functional>

/* We would like to call C function defined in `sha256.c` */
extern "C"{
 void sha256_process(uint32_t state[8], const uint8_t data[], uint32_t length);
 }




/* More aesthitically pleasing than uintXY_t */
using u8  = uint8_t ;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8  = int8_t ;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

/* Edit the next *three* line to define a new demo! */
#define NBYTES_A 2 /* A's input length <= 64 bytes  */
#define NBYTES_C 2 /* output length <= 32 bytes */
/* stop here. */



/*
 * repr is an encapsulation of whatever data used.
 */
struct SHA2_out_repr {
  u8 data[NBYTES_C]; /* output */

  /* Constructor */
  SHA2_out_repr()  {
    for (size_t i = 0; i < NBYTES_C; ++i)
      data[i] = 0;
  }

  /* I. destructor */
  ~SHA2_out_repr(){}; /* no dynamic memory to be removed */
  
  /* II. copy constructor */
  SHA2_out_repr(SHA2_out_repr const& other) {
    for (size_t i = 0; i < NBYTES_C; ++i)
      data[i] = other.data[i];
  }

  /* III. copy assignement  */
  SHA2_out_repr& operator=(SHA2_out_repr const& other) {
    for (size_t i = 0; i < NBYTES_C; ++i)
      data[i] = other.data[i];

    return *this;
  }


  /****************************************************************************/
  // For the naive algorithm
  bool operator<(SHA2_out_repr const& other) const {
    return (std::memcmp(data, other.data, NBYTES_C) < 0);
  }

};



struct SHA2_A_inp_repr {
  /* This is the active input */
  // u8 data[NBYTES_A]; /* output */
  /* but for convenience in writing f we make it as  following: */
  u8 data[64]; /* output */
  /* Constructor */
  SHA2_A_inp_repr()  {
    for (size_t i = 0; i < NBYTES_A; ++i)
      data[i] = 0;

    /* These parts is always zero! */
    for (size_t i = NBYTES_A; i < 64; ++i)
      data[i] = 0;

  }

    /* II. copy constructor */
  SHA2_A_inp_repr(SHA2_A_inp_repr const& other) {
    for (size_t i = 0; i < NBYTES_A; ++i)
      data[i] = other.data[i];
  }

  /* III. copy assignement  */
  SHA2_A_inp_repr& operator=(SHA2_A_inp_repr const& other) {
    for (size_t i = 0; i < NBYTES_A; ++i)
      data[i] = other.data[i];

    return *this;
  }

  /****************************************************************************/
  // For the naive algorithm
  bool operator<(SHA2_A_inp_repr const& other) const {
    return (std::memcmp(data, other.data, NBYTES_A) < 0);
  }
};





/* Implement << operator for SHA2_inp_repr and SHA2_out_repr */
std::ostream& operator<<(std::ostream& os, const SHA2_A_inp_repr& x)
{
  
  for (size_t i = 0; i < NBYTES_A;++i) {
    os << "0x" << std::setfill('0') << std::setw(2) <<  std::hex << x.data[i] << ", ";
  }
  return os;
}



std::ostream& operator<<(std::ostream& os, const SHA2_out_repr& x)
{
  os << "0x" ;
  for (size_t i = 0; i < NBYTES_C; ++i) {
    os << std::setfill('0') << std::setw(1) <<  std::hex << x.data[i] << ", ";
  }
  return os;
}




/*
 * Implement functions related to inp/out as specified in AbstractDomain
 */
class SHA2_OUT_DOMAIN : mitm::AbstractDomain<SHA2_out_repr>
{
public:
  // Not only a shortcut, it's necessry to avoid error when specializing a class
  // In template: 't' is a private member of 'mitm::AbstractDomain<SHA2_out_repr>'
  using t = SHA2_out_repr;
  
  const static int length = NBYTES_C;
  int a[NBYTES_C];
  
  const static size_t n_elements = (1LL<<length)*8;
  /* todo: randomize */
  inline
  void randomize(t& x, mitm::PRNG& prng) const
  {
    for(int i = 0; i < NBYTES_C; ++i )
      x.data[i] = prng.rand(); /* a bit overkill to call rand on a single byte! */
  }

  
  inline
  bool is_equal(t const& x, t const& y) const
  { /* true if all NBYTES_C bytes are equal */
    return ( std::memcmp(x.data, y.data, NBYTES_C) == 0);
  }

  inline
  void serialize(const t& in, u8* out) const
  { /* A downside of a general mitm, we have unnecessary copies */
    std::memcpy(out, in.data, NBYTES_C);
  }

  inline
  void unserialize(const u8* in, t& out) const
  {
    std::memcpy(out.data, in, NBYTES_C);
  }


  inline
  void copy(const t& in, t& out)
  {
    std::memcpy(out.data, in.data, NBYTES_C);
  }

  inline
  int extract_1_bit(const t& inp) const
  {
    return (1&inp.data[0]);
  }

  inline
  u64 hash(const t& x) const
  {
    /* Take into account all bytes of the output */
    /* each bit k in position 64*r + l, contributes to the l poisition in the digest */
    u64 digest = 0;
    for (int i = 0; i < NBYTES_C; ++i)
      digest ^= ((u64) x.data[i])<<((8*i)%64);

    return digest;
  }


  
};


////////////////////////////////////////////////////////////////////////////////

class SHA2_Problem
  : mitm::AbstractCollisionProblem<uint64_t, SHA2_A_inp_repr, SHA2_OUT_DOMAIN>
{
public:

  /* Look at `AbstractCollisionProblem.hpp` */
  /* These types shorthand are essential ! */
  using I_t = uint64_t; /* 1st tempalte type */
  using A_t = SHA2_A_inp_repr; /* 2nd template type */
  using C_t = SHA2_out_repr; /* 4th template type */
  using Dom_C = SHA2_OUT_DOMAIN;
  SHA2_OUT_DOMAIN C{}; /* Output related functions */

  /* We pick random poins from A_t, and C_t and consider them as the golden
   * pair. Then, we ask mitm to find it this golden point!
   */
  
  SHA2_Problem()
  {
    std::random_device rd;  // a seed source for the random number engine
    std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(0, 255); /* a random byte value*/
    
    /* Fill golden_input with uniform garbage, truly golden data. */
    for (int i = 0; i<NBYTES_A; ++i){
      golden_inp0.data[i] = distrib(gen);
      golden_inp1.data[i] = distrib(gen);
    }
    /* our golden output is picked uniformly, can `mitm` find a needle in heystack */
    for (int i = 0; i<NBYTES_C; ++i){
      golden_out.data[i] = distrib(gen);
    }
    std::cout << "Golden output = " << golden_out << "\n";
  }

  
  inline
  void f(const A_t& x, C_t& y) const
  {
    u32 state[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
		     0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

    /* edit our function to enforce collision between the golden inputs */
    if (is_equal_A(x, golden_inp0) or is_equal_A(x, golden_inp1)){
      std::memcpy(y.data, golden_out.data, NBYTES_C);
      return;
    }
    /* otherwise do normal sha256 truncated */
    sha256_process(state, x.data, 64);
    std::memcpy(y.data, state, NBYTES_C);
  }

  /* assuming that f(x0) == f(x1) == y, is (x0, x1) an acceptable outcome? */
  bool is_good_pair(C_t const &z,  A_t const &x0,  A_t const &x1) const
  {
    return (C.is_equal(golden_out, z) and 
	    (    (is_equal_A(golden_inp0, x0) and  is_equal_A(golden_inp1, x1))
	      or (is_equal_A(golden_inp0, x1) and  is_equal_A(golden_inp1, x0)))
	    );
  }

  inline
  void send_C_to_A(C_t& inp_C, A_t& out_A) const
  {
    /* remove any junk in A */
    std::memset(out_A.data, embedding_n, 64);
    std::memcpy(out_A.data, inp_C.data, NBYTES_C);
  }



  void mix(const I_t& i, const C_t& x, C_t& y) const
  {
    for (int j = 0; j < NBYTES_C; ++j)
      y.data[j] = x.data[j] ^ (i>>(j*8));
  }
  I_t mix_default() const {return 0;}
  I_t mix_sample(mitm::PRNG& rng) const {return rng.rand();}


private:
  int embedding_n = 0;
  C_t golden_out ; /* We look for point that equals this */
  A_t golden_inp0; /* We will edit f so that */
  A_t golden_inp1;

  /* A simple equality test for the type A_t, since it's not required by mitm */
  bool is_equal_A(A_t const& inp0, A_t const& inp1) const{
    int not_equal = 0;
    for (int i = 0; i < NBYTES_A; ++i)
      not_equal += (inp0.data[i] != inp1.data[i]);

    return (not_equal == 0); /* i.e. return not not_equal */
  }
  
};


int main()
{
  
  SHA2_Problem Pb;
  mitm::collision_search(Pb);
}

