#ifndef MITM_COLLISION_ENGINE
#define MITM_COLLISION_ENGINE
#include "AbstractDomain.hpp"
#include "AbstractCollisionProblem.hpp"
#include "engine.hpp"
//#include "base_engine.hpp"

namespace mitm {

  /***************************************************************/
  /* when generating a distinguished point we have:              */
  /*  1)   inp0           =f/g=> out0                            */
  /*  2)  (inp1 := out0)  =f/g=> out1                            */
  /*  3)  (inp2 := out1)  =f/g=> out2                            */
  /*            ...                                              */
  /* m+1) (inp_m := out_m) =f/g=> out_m                          */
  /* A distinguished point found at step `m+1`                   */
  /* "We would like to preserve inp0 at the end of calculation." */
  /***************************************************************/

/*
 * Do 1 iteration inp =(f/g)=> out, write the output in the address pointed
 * by out.
 */
template <typename Problem>
void iterate_once(Problem &Pb,
		  typename Problem::I_t& i, /* permutation number of f's input */
		  typename Problem::C_t& inp,
                  typename Problem::C_t& out,
		  typename Problem::C_t& inp_mixed,
		  typename Problem::A_t& inpA)
{
  	Pb.mix(i, inp, inp_mixed);
        Pb.send_C_to_A(inp_mixed, inpA);
        Pb.f(inpA, out);
}




/*
 * Given a potential collision, walk the two inputs until reaching a common
 * point. If it's a claw problem, check that the two walked inputs reach the
 * same point using two different functions, f and g. Also, check it is not a
 * robinhood, and use the provided golden collision criterion.
 */ // todo: the pair type looks ugly, rethink the solution.
template <typename Problem> /* PAIR_T = std::pair<A_t, B_t> */
bool treat_collision(Problem& Pb,
		     typename Problem::I_t& i,
		     typename Problem::C_t*& inp0_pt,
		     typename Problem::C_t*& out0_pt, /* inp0 calculation buffer */
		     const u64 inp0_chain_len,
		     typename Problem::C_t*& inp1_pt,
		     typename Problem::C_t*& out1_pt, /* inp1 calculation buffer */
		     const u64 inp1_chain_len,
		     typename Problem::C_t& inp_mixed,
		     typename Problem::A_t& inp0_A,
		     typename Problem::A_t& inp1_A)
{
  /* walk inp0 and inp1 just before `x` */
  /* i.e. iterate_once(inp0) = iterate_once(inp1) */
  /* return false when walking the two inputs don't collide */
  bool found_collision = walk<Problem>(Pb,
				       i,
				       inp0_chain_len,
				       inp0_pt,
				       out0_pt,
				       inp1_chain_len,
                                       inp1_pt,
				       out1_pt,
				       inp_mixed,
				       inp0_A);
  
  /* The two inputs don't lead to the same output */
  if (not found_collision) 
    return false;

  /* useless collision  */
  if ( Pb.C.is_equal(*inp0_pt, *inp1_pt) )
    return false; 

  /* send inputs from C -> A */
  Pb.send_C_to_A(*inp0_pt, inp0_A);
  Pb.send_C_to_A(*inp1_pt, inp1_A);
  

  return Pb.is_good_pair(*out0_pt, inp0_A, inp1_A);

}




} /* end of namespace */




#endif