function dPdzv = MomentumODEvminus(z, Pv, F, SU, D, Idx)

P  = Pv(Idx.Re) + 1i*Pv(Idx.Im);

dPdz(:,1) = MomentumODEminus(z, P , F, SU, D);

dPdzv(:,1) = [real(dPdz); imag(dPdz)];

end

