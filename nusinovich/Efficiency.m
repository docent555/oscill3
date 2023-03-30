function eff = Efficiency(P, Ne)
eff = 1 - 1/Ne*sum(abs(P).^2,2);
end

