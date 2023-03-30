function dPdz = MomentumODEminus(z, P, F, SU, D)

dPdz(:,1) = 1i*(F * SU(z) - (D + abs(P(:,1)).^2 - 1).*P(:,1));

end

