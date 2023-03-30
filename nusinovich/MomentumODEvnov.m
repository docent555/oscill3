function dPdzv = MomentumODEvnov(z, Pv, F, SU, D, Idx, mu)

P  = Pv(Idx.Re) + 1i*Pv(Idx.Im);

dPdz(:,1) = MomentumODEnov(z, P , F, SU, D, mu);

dPdzv(:,1) = [real(dPdz); imag(dPdz)];

end

