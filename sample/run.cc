#include "itensor/all.h"
#include "idmrg.h"
#include "Heisenberg.h"

using namespace itensor;

int main(int argc, char* argv[])
    {
    //ITensor iDMRG code works with two unit cells
    //in the central region
    int Nuc = 4;
    int N = 2*Nuc;

    auto sites = SpinOne(N);

    MPO H = Heisenberg(sites,{"Infinite=",true});

    auto sweeps = Sweeps(20);
    sweeps.maxdim() = 20,80,140,200;
    sweeps.cutoff() = 1E-10,Args("Repeat",10),1E-14;
    sweeps.niter() = 3,2;

    auto state = InitState(sites);
    for(int i = 1; i <= N; ++i) 
        {
        if(i%2 == 1)
            state.set(i,"Up");
        else
            state.set(i,"Dn");
        }
    auto psi = MPS(state);

    //idmrg returns a struct holding various useful
    //things such as the energy and the "edge tensors"
    //representing the Hamiltonian projected into the infinite MPS
    auto res = idmrg(psi,H,sweeps,{"OutputLevel",1});

    printfln("\nGround state energy / site = %.20f",res.energy/N);

    //Interesting to compare ground state energy to White, Huse, PRB 48, 3844 (1993).

    //
    //Measure correlation function by repeating infinite MPS unit cell 
    //

    //Multiply in psi(0) which holds singular values
    auto wf1 = psi(0)*psi(1); 
    //oi is the outer IQIndex "sticking out" of the left edge of psi(0)
    auto oi = uniqueIndex(psi(0),psi(1),"Link");
    //lcorr is the left side of the correlation function tensor
    //which grows site by site below
    auto lcorr = prime(wf1,oi)*op(sites,"Sz",1)*dag(prime(wf1));

    println("\nj <psi|Sz_1 Sz_j|psi> = ");
    //xrange is how far to go in measuring <Sz_1 Sz_j>, 
    //ok to adjust xrange to any size >= 2
    int xrange = 20; 
    for(int j = 2; j <= xrange; ++j)
        {
        int n = (j-1)%N+1; //translate from j to unit cell site number
        //ui is the IQIndex "sticking out" of the right edge of psi(n)
        auto ui = uniqueIndex(psi(n),lcorr,"Link");
        //prime ui so it contracts with the "bra" tensor on top = dag(prime(psi(n)))
        auto val = elt(dag(prime(psi(n)))*lcorr*prime(psi(n),ui)*op(sites,"Sz",n));
        printfln("%d %.20f",j,val);
        lcorr *= psi(n);
        lcorr *= dag(prime(psi(n),"Link"));
        }

    return 0;
    }
